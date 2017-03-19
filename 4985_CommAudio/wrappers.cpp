/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	wrappers.cpp
--
--	FUNCTIONS:      int startWinsock()
--
--	DATE:			March 3, 2017
--
--	DESIGNERS:		Robert Arendac
--
--	PROGRAMMERS:	Robert Arendac
--
--	NOTES:
--     A group of API calls wrapped up in functions.
---------------------------------------------------------------------------------------*/

#include "wrappers.h"
#include <stdio.h>

/*--------------------------------------------------------------------------------------
-- INTERFACE:     int startWinsock()
--                     VOID
--
-- RETURNS:       0 on failure, 1 on success
--
-- DATE:          March 3, 2017
--
-- DESIGNER:      Robert Arendac
--
-- PROGRAMMER:    Robert Arendac
--
-- NOTES:
-- Starts a winsock session to enable use of the Winsock DLL
---------------------------------------------------------------------------------------*/
int startWinsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        perror("WSAStartup failed");
        WSACleanup();
        return 0;
    }

    return 1;
}

int bindSocket(SOCKET s, SOCKADDR_IN *addr) {
    if (bind(s, (PSOCKADDR)addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
        fprintf(stderr, "bind failed with error %d\n", WSAGetLastError());
        return 0;
    }

    return 1;
}

int listenConnection(SOCKET s) {
    if (listen(s, 5)) {
        fprintf(stderr, "listen failed with error %d\n", WSAGetLastError());
        return 0;
    }

    return 1;
}

int connectHost(char *host) {
    if (gethostbyname(host) == NULL) {
        fprintf(stderr, "Unknown Server Address\n");
        return 0;
    }

    return 1;
}

int connectToServer(SOCKET s, SOCKADDR_IN *addr) {
    if (connect(s, (SOCKADDR *)addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
        fprintf(stderr, "Cannot connect to server, error: %d\n", WSAGetLastError());
        return 0;
    }

    return 1;
}

int acceptingSocket(SOCKET *acceptSocket, SOCKET listenSocket, SOCKADDR *addr) {

    if ((*acceptSocket = accept(listenSocket, addr, sizeof(*addr))) == NULL)
        return 0;

    return 1;
}

SOCKET createSocket(int type, int protocol) {
    SOCKET s;

    if ((s = WSASocket(AF_INET, type, protocol, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
        perror("Socket creation failed");
        return NULL;
    }

    return s;
}
