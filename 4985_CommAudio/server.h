#ifndef SERVER_H
#define SERVER_H

#include <WinSock2.h>
#include <Windows.h>
#include "serverwindow.h"
#include "wrappers.h"

void runTCPServer(ServerWindow *sw, int port);
void runUDPServer(ServerWindow *sw, int port);
SOCKADDR_IN serverCreateAddress(int port);
DWORD WINAPI tcpClient(void *arg);

#endif // SERVER_H
