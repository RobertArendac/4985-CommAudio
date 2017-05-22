/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	wrappers.cpp
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
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
    {
        perror("WSAStartup failed");
        WSACleanup();
        return 0;
    }

    return 1;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     int bindSocket(SOCKET s, SOCKADDR_IN *addr)
--                     SOCKET s: Socket to bind
--                     SOCKADDR_IN *addr: Address to bind to
--
--  RETURNS:       0 on failure, 1 on success
--
--  DATE:          March 19, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac
--
--  NOTES:
--      Wraps the bind() call.  Will return success/failure
---------------------------------------------------------------------------------------*/
int bindSocket(SOCKET s, SOCKADDR_IN *addr) {
    if (bind(s, (PSOCKADDR)addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
        qDebug() << "Socket Error: " << WSAGetLastError();
        return 0;
    }

    return 1;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     int listenConnection(SOCKET s)
--                     SOCKET s: Socket to listen on
--
--  RETURNS:       0 on failure, 1 on success
--
--  DATE:          March 19, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac
--
--  NOTES:
--      Wraps the listen() call.  Will return success/failure
---------------------------------------------------------------------------------------*/
int listenConnection(SOCKET s) {
    if (listen(s, 5)) {
        fprintf(stderr, "listen failed with error %d\n", WSAGetLastError());
        return 0;
    }

    return 1;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     int connectHost(const char *host)
--                     const char *host: Host client wishes to connect to
--
--  RETURNS:       0 on failure, 1 on success
--
--  DATE:          March 19, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac
--
--  NOTES:
--      Checks to see if the host entered by the client is a valid one.
---------------------------------------------------------------------------------------*/
int connectHost(const char *host) {
    if (gethostbyname(host) == NULL) {
        fprintf(stderr, "Unknown Server Address\n");
        return 0;
    }

    return 1;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     int connectToServer(SOCKET s, SOCKADDR_IN *addr)
--                     SOCKET s: Socket to connect on
--                     SOCKADDR_IN *addr: Address to connect to
--
--  RETURNS:       0 on failure, 1 on success
--
--  DATE:          March 19, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac
--
--  NOTES:
--      Wraps the connect() call.  Will return success/failure
---------------------------------------------------------------------------------------*/
int connectToServer(SOCKET s, SOCKADDR_IN *addr) {
    if (connect(s, (SOCKADDR *)addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
        fprintf(stderr, "Cannot connect to server, error: %d\n", WSAGetLastError());
        return 0;
    }

    return 1;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     int acceptingSocket(SOCKET *acceptSocket, SOCKET listenSocket, SOCKADDR *addr)
--                     SOCKET *acceptSocket: Socket to accept a connection on
--                     SOCKET listenSocket: Listening socket that received connection request
--                     SOCKADDR *addr: Address to connect to
--
--  RETURNS:       0 on failure, 1 on success
--
--  DATE:          March 19, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac
--
--  NOTES:
--      Wraps the accept() call.  Will return success/failure
---------------------------------------------------------------------------------------*/
int acceptingSocket(SOCKET *acceptSocket, SOCKET listenSocket, SOCKADDR *addr)
{
    int size = sizeof(*addr);
    if ((*acceptSocket = accept(listenSocket, addr, &size)) == NULL)
    {
        fprintf(stderr, "accept() failed: %d", WSAGetLastError());
        return 0;
    }

    return 1;
}

int setServOptions(SOCKET sck, int option, char *optval)
{
    int optSize = sizeof(optval);
    if (setsockopt(sck, IPPROTO_IP, option, optval, optSize) != 0)
    {
        fprintf(stderr, "setsockopt() failed: %d", WSAGetLastError());
        return 0;
    }

    return 1;
}

int setCltOptions(SOCKET sck, int option, char *optval)
{
    int optSize = sizeof(optval);
    if (setsockopt(sck, SOL_SOCKET, option, optval, optSize) != 0)
    {
        fprintf(stderr, "setsockopt() failed: %d", WSAGetLastError());
        return 0;
    }

    return 1;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     SOCKET createSocket(int type, int protocol)
--                     int type: Socket stream or datagrams
--                     int protocol: UDP or TCP
--
--  RETURNS:       NULL on failure, new socket on success
--
--  DATE:          March 19, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac
--
--  NOTES:
--      Creates a new socket for either UDP or TCP connections
---------------------------------------------------------------------------------------*/
SOCKET createSocket(int type, int protocol)
{
    SOCKET s;

    if ((s = WSASocket(AF_INET, type, protocol, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
        perror("Socket creation failed");
        return NULL;
    }

    return s;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     int recvData(SocketInformation *si, DWORD *flags, LPWSAOVERLAPPED_COMPLETION_ROUTINE routine)
--                     SocketInformation *si: struct to hold socket info
--                     DWORD *flags: Receive flags
--                     LPWSAOVERLAPPED_COMPLETION_ROUTINE routine: Completion routine for reading
--
--  RETURNS:       0 on failure, 1 on success
--
--  DATE:          April 21, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac
--
--  NOTES:
--      Performs an overlapped read.
---------------------------------------------------------------------------------------*/
int recvData(SocketInformation *si, DWORD *flags, LPWSAOVERLAPPED_COMPLETION_ROUTINE routine)
{
    WSAEVENT event[1];

    if (WSARecv(si->socket, &(si->dataBuf), 1, NULL, flags, &(si->overlapped), routine) == SOCKET_ERROR)
    {
        qDebug() << "WSARecv failed: error " << WSAGetLastError();
        return 0;
    }

    event[0] = WSACreateEvent();

    if (WSAWaitForMultipleEvents(1, event, FALSE, WSA_INFINITE, TRUE) != WAIT_IO_COMPLETION)
    {
        qDebug() << "WaitForMultipleEvents() failed: " << WSAGetLastError();
        return 0;
    }

    return 1;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     int sendData(SocketInformation *si, LPWSAOVERLAPPED_COMPLETION_ROUTINE routine)
--                     SocketInformation *si: struct to hold socket info
--                     LPWSAOVERLAPPED_COMPLETION_ROUTINE routine: Completion routine for sending
--
--  RETURNS:       0 on failure, 1 on success
--
--  DATE:          April 21, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac
--
--  NOTES:
--      Performs an overlapped send.
---------------------------------------------------------------------------------------*/
int sendData(SocketInformation *si, LPWSAOVERLAPPED_COMPLETION_ROUTINE routine)
{
    WSAEVENT event[1];

    if (WSASend(si->socket, &(si->dataBuf), 1, NULL, 0, &(si->overlapped), routine) == SOCKET_ERROR)
    {
        qDebug() << "WSASend failed: error " << WSAGetLastError();
        return 0;
    }

    event[0] = WSACreateEvent();

    if (WSAWaitForMultipleEvents(1, event, FALSE, WSA_INFINITE, TRUE) != WAIT_IO_COMPLETION)
    {
        qDebug() << "WaitForMultipleEvents() failed: " << WSAGetLastError();
        return 0;
    }

    return 1;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void resetBuffers(SocketInformation *si)
--                     SocketInformation *si: struct that holds socket info
--
--  RETURNS:       void
--
--  DATE:          April 21, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac
--
--  NOTES:
--      Resets everything in the SocketInformation struct
---------------------------------------------------------------------------------------*/
void resetBuffers(SocketInformation *si)
{
    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));
    si->bytesReceived = 0;
    si->bytesSent = 0;
    si->dataBuf.len = BUF_SIZE;
    si->dataBuf.buf = si->buffer;
}
