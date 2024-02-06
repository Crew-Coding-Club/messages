

#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>

#include "../utils/socket_utils.h"


// #define SIZE_PACKET 512
#define UDP_PORT 54454



void get_args(int argc, char *argv[], char **ip_server, int *port_server) {
    if (argc != 3) {
        fprintf(stderr, "[Error] Format : %s <ip_server> <port_server>\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    *ip_server = argv[1];
    sscanf(argv[2], "%d", port_server);

    if (!isValidPort(*port_server)) {
        fprintf(stderr, "invalid port : %d < 0", *port_server);
        exit(EXIT_FAILURE);
    }

    if (!isValidIp(*ip_server)) {
        fprintf(stderr, "ip %s invalid", *ip_server);
        exit(EXIT_FAILURE);
    }

    if (errno != 0) {
        perror("echec get args");
        exit(EXIT_FAILURE);
    }
}

int connect_to_server(struct sockaddr_in sever_addr) {
    Sock sock;
    

    /* création socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket creation error");
        return EXIT_FAILURE;
    }

    /* connection au server */
    int retour = connect(sock, (const struct sockaddr*)&sever_addr, sizeof(sever_addr));
    if ( retour == -1) {
        perror("Connect error");
        close(sock);
        return EXIT_FAILURE;
    }

    return sock;
}



struct sockaddr_in openNewPort(struct sockaddr_in addr_server) {
    /* création du socket */
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Echec creation socket");
        exit(EXIT_FAILURE);
    }

    /* envoie du message */
    sleep(1);
    Packet msg = {"UDP message"};
    printf("[client] Envoie \"%s\" in UDP to ", msg.data);
    displayAddress(addr_server, "");
    if (sendto(sockfd, (void*)&msg, sizeof(Packet), 0, (const struct sockaddr *)&addr_server, sizeof(addr_server)) < 0) {
        perror("Echec envoie message");
        exit(EXIT_FAILURE);
    }
    

    /* récupération configuration socket */
    struct sockaddr_in localAddr;
    socklen_t addrLen = sizeof(localAddr);
    if (getsockname(sockfd, (struct sockaddr *)&localAddr, &addrLen) == -1) {
        perror("Echec getsockname");
        exit(EXIT_FAILURE);
    }

    /* renvoie de l'adresse */
    return localAddr;

}



int switchToServerMode(char* server_ip, int server_port) {
    /* Ouverture d'un port libre */
    struct sockaddr_in addr_in = createAddress(server_ip, UDP_PORT);
    addr_in = openNewPort(addr_in);

    /* Crée un socket serveur */
    int socket_in = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_in < 0) {
        perror("Erreur lors de la création du socket serveur");
        exit(EXIT_FAILURE);
    }
    

    /* Lie le socket à l'adresse et au port */
    if (bind(socket_in, (struct sockaddr *)&addr_in, sizeof(addr_in)) < 0) {
        perror("Erreur lors du bind");
        exit(EXIT_FAILURE);
    }

    /* Commence à écouter sur le socket */
    if (listen(socket_in, 1) < 0) {
        perror("Erreur lors de l'écoute");
        exit(EXIT_FAILURE);
    }

    return socket_in;
}

int connectToClient(struct sockaddr_in addr) {
    return connect_to_server(addr);
}


bool getMode(int server_socket, struct sockaddr_in *client_addr) {
    Packet packet;
    recvPacket(server_socket, &packet);
    int mode;
    sscanf(packet.data, "%d", &mode);

    /* récupération addresse client */
    if(mode!=0) {
        char ip[16];
        int port;
        sscanf(packet.data+2, "%[^:]:%d", ip, &port);
        *client_addr = createAddress(ip, port);
        displayAddress(*client_addr, "adresse client to connect");
    } else {
        printf("[client] mode 0\n");
    }
    return mode==0;
}



int getSocketPort(int sock) {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    if (getsockname(sock, (struct sockaddr *)&addr, &addr_len) == -1) {
        perror("getsockname a échoué");
        exit(EXIT_FAILURE);
    }
    int port = ntohs(addr.sin_port);
    return port;
}

void socketToString(int sock, char* buff) {
    struct sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);

    if (getsockname(sock, (struct sockaddr *)&addr, &addrLen) == -1) {
        perror("getsockname");
        exit(EXIT_FAILURE);
    }

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ipStr, sizeof(ipStr));

    sprintf(buff, "%d %s", ntohs(addr.sin_port), ipStr);
}


/* ====================================================================== */
/*                    Fonction principale                                 */
/* ====================================================================== */
int main(int argc, char **argv) {
    char * ip_server;
    int port_server;
    int server_socket, client_sock;
    char msg[32] = "Hello server !";
    Packet packet;

    printf("[client] recovery of arguments...\n");
    get_args(argc, argv, &ip_server, &port_server);

    printf("[client] connexion to server %s:%d...\n", ip_server, port_server);
    struct sockaddr_in server_addr = createAddress(ip_server, port_server);
    server_socket = connect_to_server(server_addr);

    printf("[client] sending message \"%s\" to server...\n", msg);
    strncpy(packet.data, msg, DATA_SIZE);
    sendPacket(server_socket, &packet);

    printf("[client] Waiting message from server...\n");
    recvPacket(server_socket, &packet);
    printf("[client] received message \"%s\" from server\n", packet.data);


    bool server_mode;

    struct sockaddr_in client_addr;
    server_mode = getMode(server_socket, &client_addr);

    if(server_mode) {
        displaySocketAddress(server_socket, "server_socket");
        
        /* passage en mode server */
        printf("[client] Passage mode server...\n");
        int socket_in;
        socket_in = switchToServerMode(ip_server, port_server);

        // /* envoie adresse au serveur */
        // printf("[client] envoie adresse au serveur...\n");
        // socketToString(socket_in, packet);

        /* attente de connexion */
        printf("[client] Attente du client on ");
        displaySocketAddress(socket_in, "");
        struct sockaddr client_addr;
        socklen_t addr_len = sizeof(client_addr);
        client_sock = accept(socket_in, (struct sockaddr*)&client_addr, &addr_len);

        /* communication */
        printf("[client] communication avec le client...\n");
        recvPacket(client_sock, &packet);
        printf("[client] message reçu : %s\n", packet.data);

    } else {
        printf("[client] Passage mode client...\n");
        printf("[client] Connexion au client...\n");
        client_sock = connectToClient(client_addr);

        printf("[client] Envoie d'un message...\n");
        strncpy(packet.data, "bonjour amigo !", DATA_SIZE);
        sendPacket(client_sock, &packet);

    }

    printf("[client] Fermeture connexion server/clients...\n");
    close(client_sock);
    close(server_socket);

    printf("[client] End of program\n");

    return 0;
}

