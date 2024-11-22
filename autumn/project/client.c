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
#include <pthread.h>

void *client_func(void *arg) {
    int num = *(int *) arg;
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

    char *filename = "file";
    char *host = "google.com";
    char request[8096];
    memset(request, 0, 8096);
    snprintf(request, 8096,
             "GET http://%s/file?filename=%s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n",
             host,
             filename,
             host);

    if (write(client_socket, request, strlen(request)) == -1) {
        perror("write");
    }

    int BUFF_SIZE = 8096;
    char buff[BUFF_SIZE + 1];
    time_t start = time(NULL);

    int total_read = 0;
    int total_write = 0;
    int num_read;
    while ((num_read = read(client_socket, buff, BUFF_SIZE)) > 0) {
        total_read += num_read;
    }

    if (num_read == -1) {
        perror("read");
    }
    printf("thread %d done in %ld, read %d bytes, written: %d bytes\n", num, (time(NULL) - start), total_read,
           total_write);
    return NULL;
}


int main(int argc, char **argv) {
    int num = 10;
    int args[num];
    pthread_t clients[num];
    for (int i = 0; i < num; i++) {
        args[i] = i;
        pthread_create(&clients[i], NULL, client_func, &args[i]);
    }
    for (int i = 0; i < num; i++) {
        pthread_join(clients[i], NULL);
    }
}