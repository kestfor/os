#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

void handle_client(int client_socket) {
    int buff_size = 1024;
    char buff[buff_size];
    size_t number_read;
    size_t number_written;

    while (true) {
        memset(buff, 0, buff_size);

        number_read = read(client_socket, buff, buff_size);
        if (number_read == -1) {
            perror("read() failed");
            return;
        }
        printf("got message: %s\n", buff);
        printf("message was redirected\n");

        number_written = write(client_socket, buff, buff_size);
        if (number_written == -1) {
            perror("write() failed");
            return;
        }
    }
}

int main() {
    int server_socket;
    int client_socket;
    char *dsocket_file = "/tmp/dsocket";
    struct sockaddr_un server_addr;
    struct sockaddr_un client_addr;

    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket() failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, dsocket_file);

    unlink(dsocket_file);
    int err = bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));

    if (err == -1) {
        perror("bind() failed");
        close(server_socket);
        exit(1);
    }

    err = listen(server_socket, 5);

    if (err == -1) {
        perror("listen() failed");
        close(server_socket);
        exit(1);
    }

    while (true) {
        unsigned int len;
        client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &len);

        if (client_socket == -1) {
            perror("accept() failed");
            close(server_socket);
            exit(1);
        }

        int pid = fork();

        if (pid == 0) {
            close(server_socket);
            handle_client(client_socket);
            close(client_socket);
        }
    }


}