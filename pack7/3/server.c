#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define PORT 5000
#define MAX_CONNECTIONS 64
#define MAX_PENDING 4
#define SUCCESS 0
#define CONNECTION_CLOSED (-2)
#define ERROR_OCCURRED (-1)
#define FREE_FD 0

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;

int add_new_client(int *clients_fd, int new_client_fd, int max_size) {
    for (int i = 0; i < max_size; i++) {
        if (clients_fd[i] == FREE_FD) {
            clients_fd[i] = new_client_fd;
            return new_client_fd;
        }
    }
    return ERROR_OCCURRED;
}

int handle_client(int client_socket) {
    int buff_size = 1024;
    char buff[buff_size];
    size_t number_read;
    size_t number_written;

    memset(buff, 0, buff_size);

    number_read = read(client_socket, buff, buff_size);
    if (number_read == -1) {
        perror("read() failed");
        return ERROR_OCCURRED;
    }

    if (number_read == 0) {
        return CONNECTION_CLOSED;
    }

    printf("got message: %s\n", buff);

    number_written = write(client_socket, buff, buff_size);
    if (number_written == -1) {
        perror("write() failed");
        return ERROR_OCCURRED;
    }

    printf("message was redirected\n");

    return SUCCESS;
}

void handle_notifications(int server_socket) {
    fd_set file_descriptors_set;
    int client_descriptors[MAX_CONNECTIONS] = {FREE_FD};
    sockaddr_in client_addr;

    while (true) {
        int max_file_descriptor = server_socket;
        FD_ZERO(&file_descriptors_set);
        FD_SET(server_socket, &file_descriptors_set);
        for (int i = 0; i < MAX_CONNECTIONS; i++) {
            int client_descriptor = client_descriptors[i];
            if (client_descriptor != FREE_FD) {
                if (client_descriptor > max_file_descriptor) {
                    FD_SET(client_descriptor, &file_descriptors_set);
                    max_file_descriptor = client_descriptor;
                }
            }
        }

        int err = select(max_file_descriptor + 1, &file_descriptors_set, NULL, NULL, NULL);

        if (err == -1) {
            perror("select() failed()");
            return;
        }

        if (FD_ISSET(server_socket, &file_descriptors_set)) {
            unsigned int len;
            int client_socket = accept(server_socket, (sockaddr *) &client_addr, &len);

            if (client_socket == -1) {
                perror("accept() failed");
                continue;
            }

            if (add_new_client(client_descriptors, client_socket, MAX_CONNECTIONS) == ERROR_OCCURRED) {
                printf("can't register new client\n");
                close(client_socket);
                continue;
            } else {
                printf("added new client with fd: %d\n", client_socket);
            }
        }

        for (int i = 0; i < MAX_CONNECTIONS; i++) {
            if (client_descriptors[i] == FREE_FD) {
                continue;
            }

            int client_descriptor = client_descriptors[i];
            if (FD_ISSET(client_descriptor, &file_descriptors_set)) {
                err = handle_client(client_descriptor);
                if (err == CONNECTION_CLOSED) {
                    client_descriptors[i] = FREE_FD;
                    close(client_descriptor);
                    printf("client with fd %d was disconnected\n", client_descriptor);
                }
            }
        }
    }
}


int main() {
    sockaddr_in server_addr;
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == -1) {
        perror("socket() failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_family = AF_INET;
    memset(server_addr.sin_zero, 0, 8);

    if (bind(server_socket, (sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("bind() failed");
        close(server_socket);
        exit(1);
    }

    if (listen(server_socket, MAX_PENDING) == -1) {
        perror("listen() failed");
        close(server_socket);
        exit(1);
    }

    handle_notifications(server_socket);

    close(server_socket);
}