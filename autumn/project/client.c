#define _XOPEN_SOURCE 700
#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h> /* getprotobyname */
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <strings.h>
#include <bits/types/struct_timeval.h>
#include <time.h>

int main(int argc, char** argv) {
    int client_socket;
    struct sockaddr_in server_addr;


    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket() failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    int err = connect(client_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));

    if (err == -1) {
        perror("connect() failed");
        close(client_socket);
        exit(1);
    }
    char *request = "GET http://yandex.ru/ HTTP/1.0\r\nHost: yandex.ru\r\nConnection: close\r\n\r\n";
    if (write(client_socket, request, 100) == -1) {
        perror("write");
    }

    char res[10000];
    memset(res, 0, 10000);
    char *start_pt = res;
    clock_t start = clock();
    int num_read;
    while ((num_read = read(client_socket, start_pt, 10000)) > 0) {
        start_pt += num_read;
    }

    if (num_read == -1) {
        perror("read");
    }

    printf("done in %ld", (clock() - start));
    FILE *out = fopen("out.txt", "w");
    fprintf(out, "%s", res);
    return 0;
}