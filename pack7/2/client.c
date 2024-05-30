#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <netdb.h>
#include <arpa/inet.h>

int main() {
    int client_socket;
    struct sockaddr_in server_addr;


    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket() failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    bzero(&(server_addr.sin_zero),8);

    int err = connect(client_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));

    if (err == -1) {
        perror("connect() failed");
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

        number_written = write(client_socket, buff, buff_size);
        if (number_written == -1) {
            perror("write() failed");
            close(client_socket);
            exit(1);
        }

        number_read = read(client_socket, buff, buff_size);
        if (number_read == -1) {
            perror("read() failed");
            close(client_socket);
            exit(1);
        }
        printf("got message: %s\n", buff);
    }


}