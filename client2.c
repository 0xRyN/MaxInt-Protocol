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

const char username[] = "Rayan";

int greetings(int socket) {
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

    greetings(socket_fd);

    int send_string_res = send(socket_fd, "MAX", 3, 0);
    if (send_string_res < 0) {
        perror("send");
        exit(1);
    }

    char buffer[100];
    int recv_res = recv(socket_fd, buffer, 100, 0);
    if (recv_res < 0) {
        perror("recv");
        exit(1);
    }

    // Let's process the received message :

    char *iterator = buffer;
    if (strncmp(iterator, "NOP", 3) == 0) {
        printf("NOP\n");
    } else {
        // Now, we read backwards the max number and convert it
        uint16_t max;
        memcpy(&max, buffer + (recv_res - 2), 2);
        max = ntohs(max);

        // We can read the IP next
        uint32_t ip;
        memcpy(&ip, buffer + (recv_res - 6), sizeof(ip));
        struct in_addr ip_addr;
        ip_addr.s_addr = ip;

        // We can now read the username :

        char user[MAX_NAME + 1];
        // We remove from the total read bytes :
        // - 2 bytes for the max number
        // - 4 bytes for the IP
        // - 3 bytes for the "REP" string

        memcpy(user, buffer + 3, (recv_res - 2 - 4 - 3));

        // Add the null terminator
        user[recv_res - 2 - 4 - 3] = '\0';

        printf("User %s (IP : %s) did the highest score with %d \n", user,
               inet_ntoa(ip_addr), max);
    }
    return 0;
}