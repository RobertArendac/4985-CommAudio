#ifndef SERVER_H
#define SERVER_H

#include <WinSock2.h>
#include <Windows.h>
#include "wrappers.h"

void runTCPServer(ServerWindow *sw, int port);
void runUDPServer(ServerWindow *sw, int port);
SOCKADDR_IN serverCreateAddress(int port);
DWORD WINAPI tcpClient(void *arg);
void CALLBACK clientRoutine(DWORD error, DWORD, LPWSAOVERLAPPED, DWORD);
void sendSongs(SocketInformation *si);

#endif // SERVER_H
