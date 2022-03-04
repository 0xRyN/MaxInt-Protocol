#include "arpa/inet.h"
#include "netdb.h"
#include "netinet/in.h"
#include "pthread.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/socket.h"
#include "sys/types.h"
#include "sys/wait.h"
#include "unistd.h"

#define MAX_NAME 10

int PORT = 8080;
int received_int_flag = 0;

typedef struct User {
    uint16_t max;
    char username[MAX_NAME];
    uint32_t ip;
} User;

typedef struct thread_params {
    int socket;
    uint32_t ip;
} thread_params;

pthread_mutex_t mutex;
User max_user = {
    .max = 0,
    .username = "",
    .ip = 0,
};

int get_username(char *dest, int sock) {
    char buffer[MAX_NAME];
    int read_bytes = recv(sock, buffer, MAX_NAME, 0);
    if (read_bytes < 0) {
        perror("recv");
        exit(1);
    }

    buffer[MAX_NAME] = '\0';

    // Update dest

    strcpy(dest, buffer);

    // Respond to client

    int res_buf_size = 6 + read_bytes;

    char res_buf[res_buf_size];
    snprintf(res_buf, res_buf_size, "Hello %s", buffer);
    int send_res = send(sock, res_buf, res_buf_size, 0);
    if (send_res < 0) {
        perror("send");
        exit(1);
    }

    return 0;
}

int receive_int(int sock, char *username, uint32_t *ip) {
    char buffer[2];
    int read_bytes = recv(sock, buffer, 2, 0);
    if (read_bytes != 2) {
        perror("recv didn't receieve an uint16_t");
        exit(1);
    }
    // If flag is 0, set it to 1
    if (!received_int_flag) {
        received_int_flag = 1;
    }
    uint16_t int_val;
    memcpy(&int_val, buffer, 2);
    int_val = ntohs(int_val);
    printf("-----------------\n");
    printf("Integer received : %d\n", int_val);
    printf("Current max : %d\n", max_user.max);
    if (int_val > max_user.max) {
        pthread_mutex_lock(&mutex);
        // Update max and user info
        max_user.max = int_val;
        max_user.ip = *ip;
        strcpy(max_user.username, username);
        pthread_mutex_unlock(&mutex);
        printf("Updated max to %d\n", max_user.max);
    }
    printf("-----------------\n\n");

    return 0;
}

int respond_max(int sock) {
    if (!received_int_flag) {
        printf("No max received yet\n");
        send(sock, "NOP", 3, 0);

    } else {
        char buffer[100];
        int offset = 0;
        // Convert MAX to big endian
        printf("Sending max : %d\n", max_user.max);
        uint16_t max = htons(max_user.max);
        memmove(buffer + offset, "RES", 3);
        offset += 3;

        memmove(buffer + offset, max_user.username, strlen(max_user.username));
        offset += strlen(max_user.username);

        memmove(buffer + offset, &max_user.ip, 4);
        offset += 4;

        memmove(buffer + offset, &max, 2);
        offset += 2;

        send(sock, buffer, offset, 0);
    }

    return 0;
}

int interact(int sock, char *username, uint32_t *ip) {
    char operation[4];
    int read_op = recv(sock, operation, 4, 0);
    if (read_op < 0) {
        perror("recv");
        return -1;
    }

    operation[3] = '\0';

    int cmp_res = strcmp(operation, "INT");
    if (cmp_res == 0) {
        // We have recieved an INT message
        int res = receive_int(sock, username, ip);
        if (res < 0) {
            return -1;
        } else {
            int res_send = send(sock, "INTOK", 5, 0);
            if (res_send < 0) {
                perror("send");
                return -1;
            }
            close(sock);
        }
    }

    else {
        int res = respond_max(sock);
        if (res < 0) {
            return -1;
        }
    }

    return 0;
}

void *handle_client(void *args) {
    thread_params *params = (thread_params *)args;
    int sock = params->socket;
    uint32_t *ip = malloc(sizeof(uint32_t));
    *ip = params->ip;
    free(args);

    char *username = malloc(MAX_NAME);
    get_username(username, sock);

    interact(sock, username, ip);

    free(ip);
    free(username);
    return NULL;
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
    pthread_mutex_init(&mutex, NULL);
    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sock < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in serv_addr;

    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    int listen_res = listen(serv_sock, 0);
    if (listen_res < 0) {
        perror("listen");
        exit(1);
    }

    while (1) {
        struct sockaddr_in cli_addr;
        socklen_t cli_addr_len = sizeof(cli_addr);
        int *cli_sock = malloc(sizeof(int));
        *cli_sock =
            accept(serv_sock, (struct sockaddr *)&cli_addr, &cli_addr_len);
        if (*cli_sock < 0) {
            perror("accept");
            exit(1);
        }

        pthread_t thread;
        thread_params *params = malloc(sizeof(thread_params));
        params->socket = *cli_sock;
        params->ip = cli_addr.sin_addr.s_addr;
        free(cli_sock);
        pthread_create(&thread, NULL, handle_client, params);
    }
    pthread_mutex_destroy(&mutex);
    return 0;
}