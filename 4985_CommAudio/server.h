#ifndef SERVER_H
#define SERVER_H

#include <WinSock2.h>
#include <Windows.h>

void runTCPServer(ServerWindow *sw);
SOCKADDR_IN serverCreateAddress(int port);

#endif // SERVER_H
