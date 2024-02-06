#define _POSIX_C_SOURCE 200809L

#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../utils/socket_utils.h"

#define MAX_NB_CLIENTS 2
#define UDP_PORT 54454


// char *ip;


void get_args(int argc, char *argv[], int *port_server) {
    if (argc != 2) {
        fprintf(stderr, "[Error] Format : %s <ip_server> <port_server>\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    sscanf(argv[1], "%d", port_server);

    if (!isValidPort(*port_server)) {
        fprintf(stderr, "invalid port : %d < 0", *port_server);
        exit(EXIT_FAILURE);
    }

    if (errno != 0) {
        perror("echec get args");
        exit(EXIT_FAILURE);
    }
}


int openUDPPort(struct sockaddr_in addr) {
    int sockfd;
    
    /* ouverture socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Echec de la création du socket");
        exit(EXIT_FAILURE);
    }

    /* bind socket au port */
    if (bind(sockfd, (const struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Echec du bind");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}


struct sockaddr_in receiveAddrInUDP(int sockfd) {
    struct sockaddr_in client_addr;
    Packet packet;
    printf("En attente de paquets UDP sur : ");
    displaySocketAddress(sockfd, "socket server UDP");

    /* réception paquet */
    socklen_t len = sizeof(client_addr);
    int message_len = recvfrom(sockfd, (void *)&packet, sizeof(Packet), 0, (struct sockaddr *)&client_addr, &len);
    if (message_len < 0) {
        perror("Echec de recvfrom");
        exit(EXIT_FAILURE);
    }

    displayAddress(client_addr, "adresse source du paquet UDP");

    return client_addr;
}


void connectClients(int socketClient0, int socketClient1) {
    struct sockaddr_in udp_addr, client0_addr;
    char str_client0_addr[32];
    int udp_socket;
    Packet packet;

    /* création sockect UDP*/
    udp_addr = createAddress(NULL, UDP_PORT);
    udp_socket = openUDPPort(udp_addr);

    /* envoie client 0 mode 0 (server)*/
    strncpy(packet.data, "0", DATA_SIZE);
    printf("[server] envoie message \"%s\" client 0...\n", packet.data);
    sendPacket(socketClient0, &packet);

    /* récupération adresse client 0 */
    printf("[server] attente de l'adresse...\n");
    client0_addr = receiveAddrInUDP(udp_socket);

    /* convertion adresse client en string */
    addrToString(client0_addr, str_client0_addr);
    

    /* envoie client 1 mode 1 + addr */
    snprintf(packet.data, DATA_SIZE, "%d %s", 1, str_client0_addr);
    printf("[server] envoie message \"%s\" client 1...\n", packet.data);
    sendPacket(socketClient1, &packet);

    /* fin de connexion avec server */
    printf("[server] fermeture connexion clients...\n");
    close(socketClient0);
    close(socketClient1);
}


int main(int argc, char **argv) {
    int port;

    get_args(argc, argv, &port);

    setsockopt(1, 1, 1, NULL, 1);

    printf("[server] creation socket...\n");
    struct sockaddr_in clients_addr[MAX_NB_CLIENTS];
    struct sockaddr_in server_addr = createAddress(NULL, port);
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

    Packet packet;
    Packet msg = {"Hello client !"};

    

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

        recvPacket(clients[i], &packet);
        printf("[server] message \"%s\" du client\n", packet.data);

        printf("[server] envoie \"%s\" au client...\n", msg.data);
        sendPacket(clients[i], &msg);


    }
    printf("[server] redirection communications...\n");
    connectClients(clients[0], clients[1]);

    printf("[server] fermeture connexion...\n");
    close(server);

    exit(EXIT_SUCCESS);
}