#ifndef CLIENT_H
#define CLIENT_H

#include <WinSock2.h>
#include <Windows.h>
#include "wrappers.h"

SOCKADDR_IN clientCreateAddress(const char *host, int port);
void runTCPClient(ClientWindow *cw, const char *ip, int port);
void runUDPClient(ClientWindow *cw, const char *ip, int port);
void CALLBACK songRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);

#endif // CLIENT_H
