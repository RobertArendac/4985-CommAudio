#include "client.h"
#include "server.h"
#include <WS2tcpip.h>

/*--------------------------------------------------------------------------------------
--  INTERFACE:     SOCKADDR_IN serverCreateAddress(const char *host, int port)
--                     const char *host: Host to connect to
--                     int port: Port to bind to
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
--      Fills an address struct.  IP is server IP, port is passed in, used for TCP.
---------------------------------------------------------------------------------------*/
SOCKADDR_IN clientCreateAddress(const char *host, int port) {
    SOCKADDR_IN addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons(port);

    return addr;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void runTCPClient(ClientWindow *cw, const char *ip, int port)
--                     ClientWindow *cw: UI to update
--                     const char *ip: Host to connect to
--                     int port: Port to bind to
--
--  RETURNS:
--
--  DATE:          March 19, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac
--
--  NOTES:
--      Will connect to a TCP server.
---------------------------------------------------------------------------------------*/
void runTCPClient(ClientWindow *cw, const char *ip, int port) {
    SOCKET sck;
    SOCKADDR_IN addr;
    DWORD recvBytes, flags = 0;
    SocketInformation *si;

    // Create a TCP socket
    if ((sck = createSocket(SOCK_STREAM, IPPROTO_TCP)) == NULL)
        return;

    // Check for a valid host
    if (!connectHost(ip))
        return;

    // Initialize address info
    memset((char *)&addr, 0, sizeof(SOCKADDR_IN));
    addr = clientCreateAddress(ip, port);

    // Connect to the server
    if (!connectToServer(sck, &addr)) {
        return;
    }

    si = (SocketInformation *)malloc(sizeof(SocketInformation));

    si->socket = sck;
    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));
    si->bytesReceived = 0;
    si->bytesSent = 0;
    si->dataBuf.len = 1024;
    si->dataBuf.buf = si->buffer;
    WSARecv(si->socket, &(si->dataBuf), 1, &recvBytes, &flags, &(si->overlapped), songRoutine);

    // TAKE OUT WHEN THERE IS MORE CLIENT CODE
    Sleep(5000);

    printf("closing socket");
    closesocket(sck);
    WSACleanup();
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void runUDPClient(ClientWindow *cw, const char *ip, int port)
--                     ClientWindow *cw: UI to update
--                     const char *ip: Host to connect to
--                     int port: Port to bind to
--
--  RETURNS:
--
--  DATE:          March 19, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac
--
--  NOTES:
--      Will set up a UDP socket for sending audio data.  Also joins a multicast group
--      in order to send audio to all connected clients.
---------------------------------------------------------------------------------------*/
void runUDPClient(ClientWindow *cw, const char *ip, int port) {
    SOCKET sck;
    SOCKADDR_IN addr, srvAddr;
    struct ip_mreq stMreq;
    int flag = 1;

    // Create a UDP socket
    if ((sck = createSocket(SOCK_DGRAM, IPPROTO_UDP)) == NULL)
        return;

    // Check for valid host
    if (!connectHost(ip))
        return;

    // Init address info
    memset((char *)&addr, 0, sizeof(SOCKADDR_IN));
    addr = clientCreateAddress(ip, port);

    // Set the reuse addr
    if (!setCltOptions(sck, SO_REUSEADDR, (char *)&flag))
        return;

    memset((char *)&srvAddr, 0, sizeof(SOCKADDR_IN));
    srvAddr = serverCreateAddress(port);

    if (!bindSocket(sck, &srvAddr))
        return;

    stMreq.imr_multiaddr.s_addr = inet_addr(MCAST_ADDR);
    stMreq.imr_interface.s_addr = INADDR_ANY;

    if (!setServOptions(sck, IP_ADD_MEMBERSHIP, (char *)&stMreq))
        return;

    // Do stuff here

    printf("closing socket");
    closesocket(sck);
    WSACleanup();
}

void CALLBACK songRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags) {
    DWORD recvBytes;

    SocketInformation *si = (SocketInformation *)overlapped;

    if (error != 0 || bytesTransferred == 0) {
        if (error) {
            fprintf(stderr, "Error: %d\n", error);
        }
        fprintf(stderr, "Closing socket: %d\n", si->socket);
        closesocket(si->socket);
        return;
    }

    fprintf(stdout, "\n%s\n", si->dataBuf.buf);
}
