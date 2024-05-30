#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

int main() {
    int client_socket;
    char *dsocket_file = "/tmp/dsocket";
    char *dsocket_file_client = "/tmp/dsocket_сlient";
    struct sockaddr_un client_addr;
    struct sockaddr_un server_addr;


    client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket() failed");
        exit(1);
    }

    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, dsocket_file);

    client_addr.sun_family = AF_UNIX;
    strcpy(client_addr.sun_path, dsocket_file_client);

    unlink(dsocket_file_client);
    int err = bind(client_socket, (struct sockaddr *) &client_addr, sizeof(client_addr));

    if (err == -1) {
        perror("bind() failed");
        close(client_socket);
        exit(1);
    }

    err = connect(client_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));

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