/* All the prototypes for the wrapper functions found in wrappers.cpp */

#ifndef WRAPPERS_H
#define WRAPPERS_H

#include <WinSock2.h>
#include <Windows.h>
#include "serverwindow.h"
#include "clientwindow.h"

#define MCAST_ADDR  "234.5.6.7"
#define MCAST_PORT  8910
#define MCAST_TTL   32  //Kind of an arbitrary setting, I don't know what's best here...

#define SONG_SIZE 1024
#define BUF_SIZE 2048
#define IP_SIZE 16

typedef struct {
    char cltIP[IP_SIZE];
    int TCPPort;
    int UDPPort;
    ServerWindow *sWindow;
    ClientWindow *cWindow;
} ThreadInfo;

// Hold all the information associated with a socket
typedef struct SocketInformation {
    OVERLAPPED overlapped;
    char buffer[BUF_SIZE];
    //char filename[BUF_SIZE];
    WSABUF dataBuf;
    SOCKET socket;
    DWORD bytesSent;
    DWORD bytesReceived;
} SocketInformation;

int startWinsock();
int bindSocket(SOCKET s, SOCKADDR_IN *addr);
int listenConnection(SOCKET s);
int connectHost(const char *host);
int connectToServer(SOCKET s, SOCKADDR_IN *addr);
int acceptingSocket(SOCKET *acceptSocket, SOCKET listenSocket, SOCKADDR *addr);
int setServOptions(SOCKET sck, int option, char *optval);
int setCltOptions(SOCKET sck, int option, char *optval);
SOCKET createSocket(int type, int protocol);

#endif // WRAPPERS_H
