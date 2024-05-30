#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main() {
    int server_socket;
    int client_socket;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == -1) {
        perror("socket() failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1025);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero),8);

    int err = bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));

    if (err == -1) {
        perror("bind() failed");
        close(server_socket);
        exit(1);
    }

    int buff_size = 1024;
    char buff[buff_size];
    while (true) {
        printf("listen for messages...\n");
        socklen_t len = sizeof(struct sockaddr);
        if (recvfrom(server_socket, buff, buff_size, 0, (struct sockaddr *) &client_addr, &len) == -1) {
            perror("recvfrom() failed");
            close(server_socket);
            exit(1);
        }

        printf("got message: %s\n", buff);

        client_addr.sin_family = AF_INET;

        if (sendto(server_socket, buff, buff_size, 0, (struct sockaddr *) &client_addr, len) == -1) {
            perror("sendto() failed");
            close(server_socket);
            exit(1);
        }

        printf("message was redirected\n");
    }


}