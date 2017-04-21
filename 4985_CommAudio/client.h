#ifndef CLIENT_H
#define CLIENT_H

#include <WinSock2.h>
#include <Windows.h>
#include "wrappers.h"

//Socket handlers
SOCKADDR_IN clientCreateAddress(const char *host, int port);
void runTCPClient(ClientWindow *cw, const char *ip, int port);
void runUDPClient(ClientWindow *cw, const char *ip, int port);

//request methods
void downloadSong(const char *song);
void requestSong(const char *song);
void updateClientSongs();
void uploadSong(QString song);

#endif // CLIENT_H
