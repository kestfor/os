#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>

int main() {
    int client_socket;
    struct sockaddr_in server_addr;

    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == -1) {
        perror("socket() failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1025);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) == -1) {
        perror("inet_pton() failed");
        exit(1);
    }
    server_addr.sin_family = AF_INET;
    bzero(&(server_addr.sin_zero),8);

    struct timeval timeOut;
    timeOut.tv_sec = 2;
    timeOut.tv_usec = 0;
    if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (void *) &timeOut, sizeof(struct timeval)) == -1) {
        perror("setsockopt() failed");
        close(client_socket);
        exit(1);
    }

    int buff_size = 1024;
    char buff[buff_size];
    size_t number_read;
    size_t number_written;

    while (true) {
        memset(buff, 0, buff_size);
        printf("enter message: ");
        char *res = fgets(buff, buff_size, stdin);
        if (res == NULL) {
            perror("fgets() failed");
            close(client_socket);
            exit(1);
        }

        if (buff[0] == '\n') {
            close(client_socket);
            break;
        }
        buff[strrchr(buff, '\n') - buff] = '\0';

        if (sendto(client_socket, buff, buff_size, 0, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
            perror("sentdo() failed");
            close(client_socket);
            exit(1);
        }

        server_addr.sin_family = AF_INET;

        socklen_t size = sizeof(struct sockaddr);

        if (recvfrom(client_socket, buff, buff_size, 0, (struct sockaddr *) &server_addr, &size) == -1) {
            perror("recvfrom() failed");
        } else {
            printf("got message: %s\n", buff);
        }
    }


}