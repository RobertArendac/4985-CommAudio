/* All the prototypes for the wrapper functions found in wrappers.cpp */

#ifndef WRAPPERS_H
#define WRAPPERS_H

#include <WinSock2.h>
#include <Windows.h>

#define MCAST_ADDR  "234.5.6.7"
#define MCAST_PORT  8910
#define MCAST_TTL   32

int startWinsock();
int bindSocket(SOCKET s, SOCKADDR_IN *addr);
int listenConnection(SOCKET s);
int connectHost(const char *host);
int connectToServer(SOCKET s, SOCKADDR_IN *addr);
int acceptingSocket(SOCKET *acceptSocket, SOCKET listenSocket, SOCKADDR *addr);
int setOptions(SOCKET sck, int option, char *optval);
SOCKET createSocket(int type, int protocol);

#endif // WRAPPERS_H
