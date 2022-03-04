#include <arpa/inet.h>

#include "netinet/in.h"
#include "pthread.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/socket.h"
#include "sys/types.h"
#include "time.h"
#include "unistd.h"

#define MAX_NAME 10
#define IP "127.0.0.1"

int PORT = 8080;

const char *usernames[] = {"Alice", "Bob", "Charlie", "David", "Eve"};

int greetings(int index, int socket) {
    char username[MAX_NAME];
    strcpy(username, usernames[index]);
    int send_res = send(socket, username, strlen(username), 0);
    if (send_res < 0) {
        perror("send");
        exit(1);
    }
    // Don't need to receive anything from client
    char buffer[100];
    int recv_res = recv(socket, buffer, 100, 0);
    if (recv_res < 0) {
        perror("recv");
        exit(1);
    }
    return 0;
}

int check_port(int argc, char *port) {
    if (argc != 2) {
        printf("Usage: ./client|server <port>\n");
        printf("Default port is 8080\n");
        return -1;
    }
    int port_int = atoi(port);
    if (port_int < 1024 || port_int > 65535) {
        printf("Port number must be between 1024 and 65535\n");
        printf("Default port is 8080\n");
        return -1;
    }
    PORT = port_int;
    return 0;
}

int main(int argc, char *argv[]) {
    check_port(argc, argv[1]);
    srand(time(NULL));
    for (int i = 0; i < 5; i++) {
        int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd < 0) {
            perror("socket");
            exit(1);
        }

        uint32_t server_ip = inet_addr(IP);

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = server_ip;

        if (connect(socket_fd, (struct sockaddr *)&server_addr,
                    sizeof(server_addr)) < 0) {
            perror("connect");
            exit(1);
        }

        greetings(i, socket_fd);

        uint16_t random_int = (rand() % 1000);
        printf("%d\n", random_int);
        random_int = htons(random_int);
        printf("now : %d\n", random_int);
        int send_string_res = send(socket_fd, "INT ", 4, 0);
        if (send_string_res < 0) {
            perror("send");
            exit(1);
        }

        int send_int_res = send(socket_fd, &random_int, sizeof(random_int), 0);
        if (send_int_res < 0) {
            perror("send");
            exit(1);
        }

        char buffer[100];
        int recv_res = recv(socket_fd, buffer, 100, 0);
        if (recv_res < 0) {
            perror("recv");
            exit(1);
        }

        printf("%s\n", buffer);
        close(socket_fd);
    }
}