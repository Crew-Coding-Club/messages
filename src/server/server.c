#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_NB_CLIENTS 3
#define SIZE_PACKET 512

void get_args(int argc, char *argv[], char **ip_server, int *port_server) {
    if (argc != 3) {
        fprintf(stderr, "[Error] Format : %s <ip_server> <port_server>\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    *ip_server = argv[1];
    sscanf(argv[2], "%d", port_server);

    if (*port_server < 0 || *port_server > 65535) {
        fprintf(stderr, "invalid port : %d < 0", *port_server);
        exit(EXIT_FAILURE);
    }

    int ip1, ip2, ip3, ip4;
    if (sscanf(*ip_server, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4) != 4) {
        fprintf(stderr, "echec lecture ip %s\n", *ip_server);
        exit(EXIT_FAILURE);
    }

    if (ip1 < 0 || ip2 < 0 || ip3 < 0 || ip4 < 0 || ip1 > 255 || ip2 > 255 ||
        ip3 > 255 || ip4 > 255) {
        fprintf(stderr, "ip %s invalid", *ip_server);
        exit(EXIT_FAILURE);
    }

    if (errno != 0) {
        perror("echec get args");
        exit(EXIT_FAILURE);
    }
}

struct sockaddr_in create_addr(char *ip, int port) {
    struct sockaddr_in addr_server;
    memset(&addr_server, 0, sizeof(addr_server));
    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &addr_server.sin_addr.s_addr) == -1) {
        perror("inet_pton error");
        exit(EXIT_FAILURE);
    }
    return addr_server;
}

void send_message(int sock, char *msg) {
    char buff[SIZE_PACKET]="";
    strncpy(buff, msg, SIZE_PACKET);
    if (send(sock, (const void *)buff, SIZE_PACKET * sizeof(char), 0) == -1) {
        perror("Send error");
    }
}

void recv_message(int sock, char *msg) {
    char buff[SIZE_PACKET]="";
    if (recv(sock, (void *)buff, SIZE_PACKET * sizeof(char), 0) == -1) {
        perror("receive error");
    }
    if (strncpy(msg, buff, SIZE_PACKET) == NULL) {
        perror("copy message error");
    }
}

int main(int argc, char **argv) {
    char *ip;
    int port;
    get_args(argc, argv, &ip, &port);

    printf("[server] creation socket...\n");
    struct sockaddr_in clients_addr[MAX_NB_CLIENTS];
    struct sockaddr_in server_addr = create_addr(ip, port);
    socklen_t server_addr_len = sizeof(server_addr);
    int clients[MAX_NB_CLIENTS];
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    printf("[server] bind socket...\n");
    if (bind(server, (const struct sockaddr *)&server_addr, server_addr_len) ==
        -1) {
        perror("binding failed");
        exit(EXIT_FAILURE);
    }

    printf("[server] listen...\n");
    if (listen(server, MAX_NB_CLIENTS) == -1) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    char buffer[SIZE_PACKET] = "";
    char msg[SIZE_PACKET] = "Hello client !";

    for (int i = 0; i < MAX_NB_CLIENTS; i++) {
        printf("[server] wait connexion...\n");
        socklen_t client_size = sizeof(clients_addr[i]);
        clients[i] =
            accept(server, (struct sockaddr *)&clients_addr[i], &client_size);
        printf("[server] new connexion...\n");
        if (clients[i] == -1) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        recv_message(clients[i], buffer);
        printf("[server] message \"%s\" du client\n", buffer);

        printf("[server] envoie \"%s\" au client...\n", msg);
        send_message(clients[i], msg);

        printf("[server] fermeture connexion...\n");
        close(clients[i]);
    }

    close(server);

    exit(EXIT_SUCCESS);
}