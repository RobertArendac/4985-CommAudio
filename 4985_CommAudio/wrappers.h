/* All the prototypes for the wrapper functions found in wrappers.cpp */

#ifndef WRAPPERS_H
#define WRAPPERS_H

#include <WinSock2.h>
#include <Windows.h>
#include "serverwindow.h"
#include "clientwindow.h"
#include "socketinformation.h"

#define MCAST_ADDR  "234.5.6.7"
#define MCAST_PORT  8910
#define MCAST_TTL   32  //Kind of an arbitrary setting, I don't know what's best here...

#define SONG_SIZE 1024



int startWinsock();
int bindSocket(SOCKET s, SOCKADDR_IN *addr);
int listenConnection(SOCKET s);
int connectHost(const char *host);
int connectToServer(SOCKET s, SOCKADDR_IN *addr);
int acceptingSocket(SOCKET *acceptSocket, SOCKET listenSocket, SOCKADDR *addr);
int setServOptions(SOCKET sck, int option, char *optval);
int setCltOptions(SOCKET sck, int option, char *optval);
SOCKET createSocket(int type, int protocol);
int sendData(SocketInformation *si, LPWSAOVERLAPPED_COMPLETION_ROUTINE routine);
int recvData(SocketInformation *si, DWORD *flags, LPWSAOVERLAPPED_COMPLETION_ROUTINE routine);

#endif // WRAPPERS_H
