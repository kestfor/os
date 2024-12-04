#define _GNU_SOURCE
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <netdb.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/procfs.h>
#include <signal.h>
#include "request.h"
#include "hashmap/hashmap.h"
#include "logger/logger.h"
#include <errno.h>

#define LOG(logger, level, format, ...) ({char msg[128]; snprintf(msg, 128, format, __VA_ARGS__); logger_message(logger, msg, level);})
#define BENCHMARK_START {struct timeval _start; gettimeofday(&_start, NULL);
#define BENCHMARK_END(res) struct timeval _stop; gettimeofday(&_stop, NULL); res = (_stop.tv_sec - _start.tv_sec) * 1000000 + _stop.tv_usec - _start.tv_usec;}

const int PORT = 80;
const int BUFF_SIZE = 4096 * 8;
const int TTL = 300;
volatile bool SHUTDOWN;

typedef struct Server {
    int socket_fd;
    HashMap *cache;
    time_t last_clean_time;
} Server;

void close_server(const Server *server) {
    close(server->socket_fd);
    free_hashmap(server->cache);
}

void *cleanup(void *args) {
    Server *s = args;
    const time_t time_step = TTL / 4;
    s->last_clean_time = time(NULL) - TTL;
    FILE *file = fopen("logs/cleaning.log", "w");
    Logger *logger = logger_create(file, INFO);

    uint64_t res;
    bool cleaned;
    while (!SHUTDOWN) {
        sleep(time_step);
        BENCHMARK_START
            cleaned = hashmap_gc_do_iter(s->cache, s->last_clean_time);
        BENCHMARK_END(res)
        if (cleaned) {
            LOG(logger, INFO, "cache was cleaned in %ld ms", res);
        } else {
            LOG(logger, INFO, "lazy cleaning performed in %ld ms", res);
        }
        s->last_clean_time += time_step;
        if (SHUTDOWN) {
            break;
        }
    }
    fclose(file);
    logger_clear(logger);
    close_server(s);
    pthread_exit(NULL);
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

    const int err = bind(server->socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));

    if (err == -1) {
        perror("bind() failed");
        close(server->socket_fd);
        abort();
    }

    server->cache = create_hashmap();
    server->last_clean_time = 0;
}

int get_requested_socket_connection(const char *hostname, const Logger *logger) {
    struct sockaddr_in sockaddr_in;
    struct timeval timeout;
    timeout.tv_sec = 3; // after 3 seconds connect() will timeout
    timeout.tv_usec = 0;

    const int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd == -1) {
        LOG(logger, ERROR, strerror(errno), NULL);
        return -1;
    }

    const struct hostent *hostent = gethostbyname(hostname);
    if (hostent == NULL) {
        LOG(logger, ERROR, strerror(errno), NULL);
        return -1;
    }
    const in_addr_t in_addr = inet_addr(inet_ntoa(*(struct in_addr *) *hostent->h_addr_list));
    if (in_addr == (in_addr_t) -1) {
        LOG(logger, ERROR, strerror(errno), NULL);
        return -1;
    }
    sockaddr_in.sin_addr.s_addr = in_addr;
    sockaddr_in.sin_family = AF_INET;
    sockaddr_in.sin_port = htons(PORT);

    setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    if (connect(socket_fd, (struct sockaddr *) &sockaddr_in, sizeof(sockaddr_in)) == -1) {
        LOG(logger, ERROR, strerror(errno), NULL);
        return -1;
    }

    return socket_fd;
}

int send_request(const int socket, const http_request *req) {
    char *str_req = to_string(req);
    const size_t len = strlen(str_req);
    if (write(socket, str_req, len) != -1) {
        free(str_req);
        return 0;
    }
    free(str_req);
    return -1;
}

typedef struct read_data_args {
    int from;
    channel *ch;
    Logger *logger;
} read_data_args;

void *read_data(const void *args) {
    const read_data_args *parsed = args;
    const int from = parsed->from;
    const Logger *logger = parsed->logger;
    channel *ch = parsed->ch;
    char buffer[BUFF_SIZE];
    size_t read_num = 0;
    size_t total_read = 0;
    channel_set_empty(ch);
    while ((read_num = read(from, buffer, BUFF_SIZE)) > 0) {
        total_read += read_num;
        channel_add(ch, buffer, read_num);
    }
    channel_set_whole(ch);
    if (read_num == -1) {
        LOG(logger, ERROR, strerror(errno), NULL);
    }
    return NULL;
}

int send_data_from_channel(channel *ch, const int client_socket, const Logger *logger) {
    int offset = 0;
    while (true) {
        char buffer[BUFF_SIZE];
        int read_num;
        const bool end = channel_read_available(ch, buffer, offset, BUFF_SIZE, &read_num);
        offset += read_num;
        if (read_num > 0) {
            if (write(client_socket, buffer, read_num) < 0) {
                LOG(logger, ERROR, "write failed", NULL);
                return -1;
            }

            LOG(logger, INFO, "sent %d bytes to client", read_num);
        } else if (!end) {
            LOG(logger, INFO, "waiting for data in channel", NULL);
            //usleep(1000);
            channel_wait_for_data(ch, offset);
            LOG(logger, INFO, "waked for data", NULL);
        } else {
            break;
        }
    }
    return 0;
}

int send_cached_data(const int client_socket, const char *str_req, HashMap *cache, const Logger *logger) {
    http_request *request = create_request(str_req);

    if (request == NULL) {
        return -1;
    }

    const char *key = to_string(request);
    cached_data data;
    bool ok;
    uint64_t res;
    BENCHMARK_START
        ok = capture_item(cache, key, &data);
    BENCHMARK_END(res)
    LOG(logger, INFO, "cache checked in %ld ms", res);

    if (ok && time(NULL) - data.cached_time < TTL) {
        // cache hit
        LOG(logger, INFO, "cache hit, sending data...", NULL);

        BENCHMARK_START
            send_data_from_channel(data.data, client_socket, logger);
            release_item(cache);
        BENCHMARK_END(res)

        LOG(logger, INFO, "sending from channel done in %ld ms", res);
        return 0;
    }
    if (ok) {
        // cache hit, but data outdated
        release_item(cache);
    }

    LOG(logger, INFO, "cache miss, connecting to host...", NULL);

    const int destination_socket = get_requested_socket_connection(request->hostname, logger);

    if (destination_socket == -1) {
        LOG(logger, WARNING, "error establishing connection to server", NULL);
        clear_request(request);
        return -1;
    }

    LOG(logger, INFO, "got host connection", NULL);
    //    insert_and_capture(cache, key, NULL, &data);

    bool inserted;
    BENCHMARK_START
        inserted = insert_item(cache, key, NULL);
        ok = capture_item(cache, key, &data);
    BENCHMARK_END(res)
    if (!ok) {
        LOG(logger, WARNING, "error capturing item", NULL);
        close(destination_socket);
        clear_request(request);
        return -1;
    }

    pthread_t tid;

    if (inserted) {
        LOG(logger, INFO, "insert to cache done in %ld ms", res);

        if (send_request(destination_socket, request) != 0) {
            clear_request(request);
            close(destination_socket);
            return -1;
        }

        channel *ch = data.data;
        read_data_args args = {destination_socket, ch};


        const int err = pthread_create(&tid, NULL, read_data, &args);

        if (err != 0) {
            LOG(logger, ERROR, strerror(errno), NULL);
            close(destination_socket);
            clear_request(request);
            return -1;
        }
    } else {
        LOG(logger, INFO, "data was inserted by another thread", NULL);
    }

    LOG(logger, INFO, "starting sending data from channel", NULL);

    BENCHMARK_START
        send_data_from_channel(data.data, client_socket, logger);
        release_item(cache);
        if (inserted) {
            pthread_join(tid, NULL);
        }
    BENCHMARK_END(res)

    LOG(logger, INFO, "sending from channel done in %ld ms", res);

    close(destination_socket);
    clear_request(request);
    return 0;
}

typedef struct handle_client_args {
    int client_socket;
    HashMap *cache;
} handle_client_args;


char *read_request(const int client_fd, const Logger *logger) {
    char *buff = malloc(BUFF_SIZE);
    if (buff == NULL) {
        LOG(logger, ERROR, strerror(errno), NULL);
        return NULL;
    }

    size_t buff_size = BUFF_SIZE;
    size_t total_read = 0;
    size_t number_read;
    while ((number_read = read(client_fd, buff + total_read, BUFF_SIZE)) > 0) {
        const size_t end_len = 4;
        const char *end = "\r\n\r\n\0";
        total_read += number_read;

        LOG(logger, INFO, "read %ld bytes of request data", total_read);

        if (buff_size == total_read) {
            char *tmp = buff;
            buff_size *= 2;
            buff = realloc(buff, buff_size);
            if (buff == NULL) {
                LOG(logger, ERROR, strerror(errno), NULL);
                free(tmp);
                return NULL;
            }
        }

        buff[total_read] = '\0';

        if (total_read > end_len && (strcmp(end, buff + total_read - 4) == 0)) {
            break;
        }

        LOG(logger, INFO, "waiting for next bytes (strcmp res: %d)", strcmp(end, buff + total_read - 4));
    }

    LOG(logger, INFO, "full request received (%ld bytes)", total_read);

    if (number_read == -1) {
        LOG(logger, ERROR, strerror(errno), NULL);
        free(buff);
        return NULL;
    }
    return buff;
}


void *handle_client(void *arg) {
    handle_client_args *parsed = arg;
    const int client_socket = parsed->client_socket;

    const int tid = gettid();
    char name[128];
    snprintf(name, 128, "logs/%d.log", tid);
    FILE *file = fopen(name, "w");
    Logger *logger = logger_create(file, INFO);

    uint64_t res;
    char *buff;
    BENCHMARK_START
        buff = read_request(client_socket, logger);
    BENCHMARK_END(res)
    if (buff == NULL) {
        LOG(logger, ERROR, "error while reading user http request", NULL);
        free(parsed);
        close(client_socket);
        logger_clear(logger);
        fclose(file);
        return NULL;
    }

    LOG(logger, INFO, "request read in %ld ms", res);


    BENCHMARK_START
        send_cached_data(client_socket, buff, parsed->cache, logger);
    BENCHMARK_END(res)

    LOG(logger, INFO, "client handled in %ld ms", res);
    close(client_socket);

    free(parsed);
    free(buff);
    logger_clear(logger);
    fclose(file);
    return NULL;
}

void listen_and_accept(Server *server) {
    int err = listen(server->socket_fd, 100);
    Logger *mainLogger = logger_create(stdout, INFO);

    pthread_t clean;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&clean, &attr, cleanup, server);

    if (err == -1) {
        LOG(mainLogger, ERROR, strerror(errno), NULL);
        close(server->socket_fd);
        abort();
    }

    struct sockaddr_in client_addr;
    pthread_t handle_thread;
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    while (!SHUTDOWN) {
        unsigned int len;
        const int client_socket = accept(server->socket_fd, (struct sockaddr *) &client_addr, &len);
        if (client_socket == -1) {
            LOG(mainLogger, WARNING, strerror(errno), NULL);
            if (SHUTDOWN) {
                break;
            }
            continue;
        }

        handle_client_args *args = malloc(sizeof(handle_client_args));
        args->client_socket = client_socket;
        args->cache = server->cache;
        err = pthread_create(&handle_thread, NULL, handle_client, args);
        if (err != 0) {
            LOG(mainLogger, WARNING, "pthread_create failed, client wasn't handled", NULL);
            close(client_socket);
            free(args);
        } else {
            LOG(mainLogger, INFO, "new client connected", NULL);
        }
    }
    LOG(mainLogger, INFO, "shutting down application, waiting for cleaner", NULL);
    if (pthread_kill(clean, SIGINT)) {
        LOG(mainLogger, ERROR, strerror(errno), NULL);
    }
    logger_clear(mainLogger);
    pthread_exit(NULL);
}

static void handleSignal(int signal) {
    SHUTDOWN = true;
}

static void registerSignal() {
    struct sigaction action = {0};
    action.sa_handler = handleSignal;
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);
}


int main() {
    registerSignal();
    Server server;
    init_server(&server);
    listen_and_accept(&server);
}
