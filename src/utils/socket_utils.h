#ifndef __SOCKET_UTILS_H__
#define __SOCKET_UTILS_H__

#define DATA_SIZE 512

typedef struct s_packet {
    char data[DATA_SIZE];
} Packet;

typedef int Sock;


struct sockaddr_in createAddress(char *ip, int port);


int sendPacket(int sock, Packet *packet);

int recvPacket(int sock, Packet *packet);

void addrToString(struct sockaddr_in addr, char* buff);


bool isValidIp(char* ip);

bool isValidPort(int port);


void displayAddress(struct sockaddr_in addr, char* text);

void displayRemoteAddress(int sock, char* text);

void displaySocketAddress(int sock, char* text);

void displayPaquet(Packet *paquet);



#endif