#ifndef CLIENT_H
#define CLIENT_H

#include <WinSock2.h>
#include <Windows.h>
#include "clientwindow.h"
#include "wrappers.h"

SOCKADDR_IN clientCreateAddress(const char *host, int port);
void runTCPClient(ClientWindow *cw, const char *ip, int port);
void runUDPClient(ClientWindow *cw, const char *ip, int port);

#endif // CLIENT_H
