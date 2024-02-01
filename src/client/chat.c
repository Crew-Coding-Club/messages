

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#define SIZE_PACKET 512

typedef int Sock;
typedef struct sockaddr_in Addr;



void get_args(int argc, char* argv[], char** ip_server, int *port_server) {
    if(argc != 3) {
        fprintf(stderr, "format : %s <ip_server> <port_server>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    *ip_server = argv[1];
    sscanf(argv[2], "%d", port_server);

    if(*port_server < 0 || *port_server > 65535) {
        fprintf(stderr, "invalid port : %d < 0", *port_server);
        exit(EXIT_FAILURE);
    }

    int ip1,ip2,ip3,ip4;
    if(sscanf(*ip_server, "%d.%d.%d.%d", &ip1, &ip2,&ip3,&ip4)!= 4) {
        fprintf(stderr, "echec lecture ip %s\n", *ip_server);
        exit(EXIT_FAILURE);
    }

    if(ip1 < 0 || ip2 < 0 || ip3 < 0 || ip4 < 0
    || ip1 > 255 || ip2 > 255 || ip3 > 255 || ip4 > 255) {
        fprintf(stderr,"ip %s invalid", *ip_server);
        exit(EXIT_FAILURE);
    }

    if(errno != 0) {
        perror("echec get args");
        exit(EXIT_FAILURE);
    }
}

Sock connect_to_server(const char* ip_server, const int port_server) {
    Sock sock;
    Addr addr_server;

    /* Adresse du server */
    memset(&addr_server, 0, sizeof(addr_server));
    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(port_server);
    if(inet_pton(AF_INET, ip_server, &addr_server.sin_addr.s_addr) == -1){
        perror("inet_pton error");
        return EXIT_FAILURE;
    }

    /* création socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket creation error");
        return EXIT_FAILURE;
    }

    /* connection au server */
    int retour = connect(sock, (const struct sockaddr*)&addr_server, sizeof(addr_server));
    if ( retour == -1) {
        perror("Connect error");
        close(sock);
        return EXIT_FAILURE;
    }

    return sock;
}

void send_message(Sock sock, char* msg) {
    char buff[SIZE_PACKET];
    strncpy(buff, msg, SIZE_PACKET);
    if(send(sock, (const void*)buff, SIZE_PACKET*sizeof(char), 0) == -1){
        perror("Send error");
    }
}

void recv_message(Sock sock, char* msg) {
    char buff[SIZE_PACKET] = "";
    if(recv(sock, (void*)buff, SIZE_PACKET*sizeof(char), 0) == -1){
        perror("receive error");
    }
    if(strncpy(msg, buff, SIZE_PACKET) == NULL) {
        perror("copy message error");
    }
}


/* ====================================================================== */
/*                    Fonction principale                                 */
/* ====================================================================== */
int main(int argc, char **argv) {
    char * ip_server, ip_client;
    int port_server, port_client;
    int sock_server, sock_client;
    char msg_send[SIZE_PACKET] = "Hello server !";
    char msg_rec[SIZE_PACKET] = "";

    printf("[client] recovery of arguments...\n");
    get_args(argc, argv, &ip_server, &port_server);

    printf("[client] connexion to server %s:%d...\n", ip_server, port_server);
    sock_server = connect_to_server(ip_server, port_server);

    printf("[client] sending message \"%s\" to server...\n", msg_send);
    send_message(sock_server, msg_send);

    printf("[client] Waiting message from server...\n");
    recv_message(sock_server, msg_rec);
    printf("[client] received message \"%s\" from server\n", msg_rec);

    printf("[client] Fermeture connexion...\n");
    close(sock_server);

    printf("[client] End of program\n");

    return 0;
}

