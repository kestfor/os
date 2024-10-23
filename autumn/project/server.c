#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <netdb.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "request.h"
#include "hashmap/hashmap.h"


const int PORT = 80;
const int BUFF_SIZE = 4096;
const int TTL = 300;

typedef struct Server {
    int socket_fd;
    HashMap *cache;
    time_t last_clean_time;
} Server;

void close_server(Server *server) {
    close(server->socket_fd);
    free_hashmap(server->cache);
}

void *cleanup(void *args) {
    Server *s = args;
    while (true) {
        s->last_clean_time = time(NULL);
        clear_old(s->cache, s->last_clean_time - TTL);
        sleep(TTL);
    }
}

void init_server(Server *server) {
    struct sockaddr_in server_addr;
    server->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->socket_fd == -1) {
        perror("socket() failed");
        abort();
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    bzero(&(server_addr.sin_zero), 8);

    int err = bind(server->socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));

    if (err == -1) {
        perror("bind() failed");
        close(server->socket_fd);
        abort();
    }

    server->cache = create_hashmap();
    server->last_clean_time = 0;

}

int get_requested_socket_connection(char *hostname) {
    struct sockaddr_in sockaddr_in;
    struct timeval timeout;
    timeout.tv_sec = 3;  // after 3 seconds connect() will timeout
    timeout.tv_usec = 0;

    int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd == -1) {
        perror("socket");
        return -1;
    }

    struct hostent *hostent = gethostbyname(hostname);
    if (hostent == NULL) {
        fprintf(stderr, "error: gethostbyname(\"%s\")\n", hostname);
        return -1;
    }
    in_addr_t in_addr = inet_addr(inet_ntoa(*(struct in_addr *) *(hostent->h_addr_list)));
    if (in_addr == (in_addr_t) (-1)) {
        fprintf(stderr, "error: inet_addr(\"%s\")\n", *(hostent->h_addr_list));
        return -1;
    }
    sockaddr_in.sin_addr.s_addr = in_addr;
    sockaddr_in.sin_family = AF_INET;
    sockaddr_in.sin_port = htons(PORT);

    setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    if (connect(socket_fd, (struct sockaddr *) &sockaddr_in, sizeof(sockaddr_in)) == -1) {
        perror("connect");
        return -1;
    }

    return socket_fd;
}

int ensure_write(int dest, char *data, size_t num) {
    ssize_t num_bytes_total, num_bytes_last;
    num_bytes_total = 0;
    while (num_bytes_total < num) {
        num_bytes_last = write(dest, data + num_bytes_total, num - num_bytes_total);
        if (num_bytes_last == -1) {
            perror("write");
            return -1;
        }
        num_bytes_total += num_bytes_last;
    }
    return 0;
}

int send_request(int socket, http_request *req) {
    char *str_req = to_string(req);
    size_t len = strlen(str_req);
    if (ensure_write(socket, str_req, len) == 0) {
        free(str_req);
        return 0;
    } else {
        free(str_req);
        return -1;
    }
}

typedef struct read_data_args {
    int from;
    channel *ch;
} read_data_args;

void *read_data(void *args) {
    read_data_args *parsed = args;
    int from = parsed->from;
    channel *ch = parsed->ch;
    char buffer[BUFF_SIZE];
    size_t read_num = 0;
    size_t total_read = 0;
    empty_channel(ch);
    while ((read_num = read(from, buffer, BUFF_SIZE)) > 0) {
        total_read += read_num;
        add_to_channel(ch, buffer, read_num);
    }
    set_whole(ch);
    if (read_num == -1) {
        perror("read");
    }
    return NULL;
}

int send_data_from_channel(channel *ch, int client_socket) {
    int offset = 0;
    while (true) {
        char buffer[BUFF_SIZE];
        int read_num;
        bool end = read_available(ch, buffer, offset, BUFF_SIZE, &read_num);
        offset += read_num;
        if (read_num > 0) {
            if (ensure_write(client_socket, buffer, read_num) != 0) {
                return -1;
            }
        } else if (!end) {
            usleep(10000);
        } else {
            break;
        }
    }
    return 0;
}

int send_cached_data(const int client_socket, const char *str_req, HashMap *cache) {
    http_request *request = create_request(str_req);

    if (request == NULL) {
        return -1;
    }

    int destination_socket = get_requested_socket_connection(request->hostname);

    if (destination_socket == -1) {
        printf("error establishing connection to server\n");
        clear_request(request);
        return -1;
    }

    char *key = to_string(request);
    cached_data data;
    bool ok = capture_item(cache, key, &data);
    if (ok && ((time(NULL) - data.cached_time) < TTL)) {
        printf("cache hit\n");
        send_data_from_channel(data.data, client_socket);
        release_item(cache, key);
        return 0;
    }
    printf("cache miss\n");
    if (send_request(destination_socket, request) != 0) {
        clear_request(request);
        close(destination_socket);
        return -1;
    }

    insert_item(cache, key, NULL);
    ok = capture_item(cache, key, &data);

    if (!ok) {
        printf("error getting cached data");
        return -1;
    }

    channel *ch = data.data;

    pthread_t tid;
    read_data_args args = {destination_socket, ch};

    pthread_create(&tid, NULL, read_data, &args);

    send_data_from_channel(data.data, client_socket);
    release_item(cache, key);
    pthread_join(tid, NULL);
    close(destination_socket);
    clear_request(request);
    return 0;
}

typedef struct handle_client_args {
    int client_socket;
    HashMap *cache;
} handle_client_args;

void *handle_client(void *arg) {
    handle_client_args *parsed = arg;
    int client_socket = parsed->client_socket;
    char buff[BUFF_SIZE];
    size_t number_read;
    memset(buff, 0, BUFF_SIZE);

    number_read = read(client_socket, buff, BUFF_SIZE);
    if (number_read == -1) {
        perror("read() failed");
        close(client_socket);
        free(parsed);
        return NULL;
    }

    send_cached_data(client_socket, buff, parsed->cache);
    close(client_socket);
    free(parsed);
    return NULL;
}

void listen_and_accept(Server *server) {
    int err = listen(server->socket_fd, 5);

    pthread_t clean;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&clean, &attr, cleanup, server);

    if (err == -1) {
        perror("listen() failed");
        close(server->socket_fd);
        abort();
    }

    struct sockaddr_in client_addr;
    pthread_t handle_thread;
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    while (true) {

        unsigned int len;
        int client_socket = accept(server->socket_fd, (struct sockaddr *) &client_addr, &len);

        if (client_socket == -1) {
            perror("accept() failed");
            continue;
        }

        handle_client_args *args = malloc(sizeof(handle_client_args));
        args->client_socket = client_socket;
        args->cache = server->cache;
        pthread_create(&handle_thread, NULL, handle_client, args);
    }

}

int main() {

    Server server;
    init_server(&server);
    listen_and_accept(&server);
}