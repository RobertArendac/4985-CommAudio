/* All the prototypes for the wrapper functions found in wrappers.cpp */

#ifndef WRAPPERS_H
#define WRAPPERS_H

#include <WinSock2.h>
#include <Windows.h>

int startWinsock();
int bindSocket(SOCKET s, SOCKADDR_IN *addr);
int listenConnection(SOCKET s);
int connectHost(char *host);
int connectToServer(SOCKET s, SOCKADDR_IN *addr);
int acceptingSocket(SOCKET *acceptSocket, SOCKET listenSocket, SOCKADDR *addr);
SOCKET createSocket(int type, int protocol);

#endif // WRAPPERS_H
