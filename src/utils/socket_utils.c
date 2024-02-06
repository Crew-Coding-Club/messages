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

#include "socket_utils.h"


void error(int exit_value, char* msg) {
    if(errno) {
        perror(msg);
    } else {
        fprintf(stderr, "%s\n", msg);
    }
    exit(exit_value);
}

struct sockaddr_in createAddress(char *ip, int port) {
#ifdef DEBUG
    /* vérification des entrées */
    if(!ip) error(EXIT_FAILURE, "null pointer ip");
    if(!isValidIp(ip)) error(EXIT_FAILURE, "not valid ip");
    if(!isValidPort(port)) error(EXIT_FAILURE, "not valid port");
#endif

    /* création de l'adresse */
    struct sockaddr_in addr_server;
    memset(&addr_server, 0, sizeof(addr_server));
    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(port);
    if(ip != NULL) {
        if (inet_pton(AF_INET, ip, &addr_server.sin_addr.s_addr) == -1)
            error(EXIT_FAILURE, "inet_pton error");
    } else {
        addr_server.sin_addr.s_addr = INADDR_ANY;
    }

    return addr_server;
}


int sendPacket(int sock, Packet *packet) {
#ifdef DEBUG
    /* vérification des entrées */
    if(!packet) error(EXIT_FAILURE, "null pointer packet");
#endif

    /* envoie du paquet */
    if (send(sock, (const void *)packet, sizeof(Packet), 0) == -1) {
        perror("Send error");
        return -1;
    }
    return 0;
}

int recvPacket(int sock, Packet *packet) {
#ifdef DEBUG
    /* vérification des entrées */
    if(!packet) error(EXIT_FAILURE, "null pointer packet");
#endif

    /* réception du paquet */
    if (recv(sock, (void *)packet, sizeof(Packet), 0) == -1) {
        perror("receive error");
        return -1;
    }
    return 0;
}

void addrToString(struct sockaddr_in addr, char* buff) {
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ipStr, sizeof(ipStr));

    sprintf(buff, "%s:%d", ipStr, ntohs(addr.sin_port));
}




bool isValidIp(char* ip) {
    if(!ip) return false;

    int ip1, ip2, ip3, ip4;
    if (sscanf(ip, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4) != 4) {
        fprintf(stderr, "echec lecture ip %s\n", ip);
        exit(EXIT_FAILURE);
    }

    return !(ip1 < 0 || ip2 < 0 || ip3 < 0 || ip4 < 0 || ip1 > 255 || ip2 > 255 || ip3 > 255 || ip4 > 255);
}

bool isValidPort(int port) {
    return port > 0 && port <= 65535;
}



void displayAddress(struct sockaddr_in addr, char* text) {
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ipStr, sizeof(ipStr));

    printf("%s -> %s:%hu\n", text, ipStr, ntohs(addr.sin_port));
}

void displayRemoteAddress(int sock, char* text) {
    struct sockaddr_in peerAddr;
    socklen_t peerAddrLen = sizeof(peerAddr);

    /* récupération adresse autre extrémitée */
    if (getpeername(sock, (struct sockaddr *)&peerAddr, &peerAddrLen) == -1) {
        perror("getpeername a échoué");
        exit(EXIT_FAILURE);
    }

    displayAddress(peerAddr, text);
}

void displaySocketAddress(int sock, char* text) {
    struct sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);

    if (getsockname(sock, (struct sockaddr *)&addr, &addrLen) == -1) {
        perror("getsockname");
        exit(EXIT_FAILURE);
    }

    displayAddress(addr, text);
}

void displayPaquet(Packet *paquet) {
    printf("PACKET : <%s>\n", paquet->data);
}