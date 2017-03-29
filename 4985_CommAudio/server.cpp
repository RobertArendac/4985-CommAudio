#include "server.h"
#include "client.h"
#include <map>
#include <WS2tcpip.h>

std::map<SOCKET, std::string> clientMap;

/*--------------------------------------------------------------------------------------
--  INTERFACE:     SOCKADDR_IN serverCreateAddress(int port)
--                     int port: port to bind to
--
--  RETURNS:       Struct containing all addressing information
--
--  DATE:          March 19, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    Robert Arendac
--
--  NOTES:
--      Fills an address struct.  IP is any, port is passed in, used for TCP.
---------------------------------------------------------------------------------------*/
SOCKADDR_IN serverCreateAddress(int port) {
    SOCKADDR_IN addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    return addr;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void runTCPServer(ServerWindow *sw, int port)
--                     ServerWindow *sw: GUI to update
--                     int port: port to bind to
--
--  RETURNS:       Struct containing all addressing information
--
--  DATE:          March 19, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac
--
--  NOTES:
--      Starts up a TCP server.  Each new client will have its own dedicated thread to
--      service it.
---------------------------------------------------------------------------------------*/
void runTCPServer(ServerWindow *sw, int port)
{
    SOCKET listenSocket, acceptSocket;
    SOCKADDR_IN addr, clientAddr;

    // Create socket for listening
    if ((listenSocket = createSocket(SOCK_STREAM, IPPROTO_TCP)) == NULL)
        return;

    // Initialize address info
    addr = serverCreateAddress(port);

    // Bind the listening socket
    if (!bindSocket(listenSocket, &addr))
        return;

    // Set socket to listen for connection
    if (!listenConnection(listenSocket))
        return;

    // Accept incoming connections and put each client on thread
    while (1) {
        if (!acceptingSocket(&acceptSocket, listenSocket, (SOCKADDR *)&clientAddr))
            return;

        sw->updateClients(inet_ntoa(clientAddr.sin_addr));
        clientMap.insert(std::pair<SOCKET, std::string>(acceptSocket, inet_ntoa(clientAddr.sin_addr)));
        printf("Client %s connected\n", inet_ntoa(clientAddr.sin_addr));

        CreateThread(NULL, 0, tcpClient, &acceptSocket, 0, NULL);
    }

}

DWORD WINAPI tcpClient(void *arg)
{
    SOCKET *clientSck = (SOCKET *)arg;
    std::string songlist;
    DWORD sendBytes;
    SocketInformation *si;
    char music[1024];
    WSAEVENT events[1];
    DWORD result;

    QStringList songs = ServerWindow::getSongs();
    for (auto song : songs)
    {
        songlist += song.toStdString() + "\n";
    }

    strcpy(music, songlist.c_str());

    si = (SocketInformation *)malloc(sizeof(SocketInformation));
    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));
    strcpy(si->buffer, music);
    si->socket = *clientSck;
    si->bytesReceived = 0;
    si->bytesSent = 0;
    si->dataBuf.buf = si->buffer;
    si->dataBuf.len = 1024;

    WSASend(si->socket, &(si->dataBuf), 1, &sendBytes, 0, &(si->overlapped), clientRoutine);

    events[0] = WSACreateEvent();
    if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
        fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);

    return 0;
}

void CALLBACK clientRoutine(DWORD error, DWORD, LPWSAOVERLAPPED, DWORD) {
    if (error) {
        fprintf(stderr, "Error: %d\n", error);
    }
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void runTCPServer(ServerWindow *sw, int port)
--                     ServerWindow *sw: GUI to update
--                     int port: port to bind to
--
--  RETURNS:       Struct containing all addressing information
--
--  DATE:          March 19, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac
--
--  NOTES:
--      Starts up a UDP server.  Will be responsible for streaming audio.  Also sets up
--      multicasting
---------------------------------------------------------------------------------------*/
void runUDPServer(ServerWindow *sw, int port)
{
    SOCKADDR_IN addr, cltDest;
    SOCKET acceptSocket;
    struct ip_mreq stMreq;
    u_long ttl = MCAST_TTL;
    int flag = 0;

    // Init address info
    addr = serverCreateAddress(port);

    // Create a socket for incomming data
    if ((acceptSocket = createSocket(SOCK_DGRAM, IPPROTO_UDP)) == NULL)
        return;

    // bind the socket
    if (!bindSocket(acceptSocket, &addr))
        return;

    stMreq.imr_multiaddr.s_addr = inet_addr(MCAST_ADDR);
    stMreq.imr_interface.s_addr = INADDR_ANY;

    if (!setServOptions(acceptSocket, IP_ADD_MEMBERSHIP, (char *)&stMreq))
        return;
    if (!setServOptions(acceptSocket, IP_MULTICAST_TTL, (char *)&ttl))
        return;
    if (!setServOptions(acceptSocket, IP_MULTICAST_LOOP, (char *)&flag))
        return;

    cltDest = clientCreateAddress(MCAST_ADDR, MCAST_PORT);

    while (1)
    {
        // Do stuff
    }
}
