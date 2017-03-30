#include "client.h"
#include "server.h"
#include <WS2tcpip.h>

ClientWindow *clientWind;

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
SOCKADDR_IN clientCreateAddress(const char *host, int port)
{
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
--  RETURNS:       0 on success
--                 -1 on socket error
--                 -2 on invalid IP error
--                 -3 on connection error
--
--  DATE:          March 19, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac, Alex Zielinski
--
--  NOTES:
--      Will connect to a TCP server. Once a connection is established, it will receive
--      a message containing the server song list.
---------------------------------------------------------------------------------------*/
int runTCPClient(ClientWindow *cw, const char *ip, int port)
{
    SOCKET sck;                 //Socket to send/receive on
    SOCKADDR_IN addr;           //Address of server
    DWORD recvBytes, flags = 0; //Used for receiving data
    SocketInformation *si;      //Struct to hold socket info
    WSAEVENT events[1];         //Event array
    DWORD result;               //Result of waiting for event

    clientWind = cw;    //Init the global ClientWindow

    // Create a TCP socket
    if ((sck = createSocket(SOCK_STREAM, IPPROTO_TCP)) == NULL)
        return -1;

    // Check for a valid host
    if (!connectHost(ip))
        return -2;

    // Initialize address info
    memset((char *)&addr, 0, sizeof(SOCKADDR_IN));
    addr = clientCreateAddress(ip, port);

    // Connect to the server
    if (!connectToServer(sck, &addr))
        return -3;

    //Allocate socket information
    si = (SocketInformation *)malloc(sizeof(SocketInformation));

    //Fill in the socket info
    si->socket = sck;
    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));
    si->bytesReceived = 0;
    si->bytesSent = 0;
    si->dataBuf.len = 1024;
    si->dataBuf.buf = si->buffer;

    // Receive data, will be the song list
    WSARecv(si->socket, &(si->dataBuf), 1, &recvBytes, &flags, &(si->overlapped), songRoutine);

    //Wait for receive to complete
    events[0] = WSACreateEvent();
    if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
        fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);

    /* This is here because we do not have a graceful shutdown.  We will need to design all sockets
     * being closed and all TCP and UDP functions ending before performing any sort of cleanup.
     */
    while(1);

    printf("closing socket");
    closesocket(sck);
    WSACleanup();

    return 0;
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
--  PROGRAMMER:    Robert Arendac
--
--  NOTES:
--      Will set up a UDP socket for sending audio data.  Also joins a multicast group
--      in order to send audio to all connected clients.
---------------------------------------------------------------------------------------*/
void runUDPClient(ClientWindow *cw, const char *ip, int port)
{
    SOCKET sck;                 //Socket to send/receive on
    SOCKADDR_IN addr, srvAddr;  //Addresses for sending/receiving
    struct ip_mreq stMreq;      //Struct for multicasting
    int flag = 1;               //True flag

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
    srvAddr = serverCreateAddress(MCAST_PORT);

    // Bind to multicast group
    if (!bindSocket(sck, &srvAddr))
        return;

    // Setup multicast interface
    stMreq.imr_multiaddr.s_addr = inet_addr(MCAST_ADDR);
    stMreq.imr_interface.s_addr = INADDR_ANY;

    // Join multicast group
    if (!setServOptions(sck, IP_ADD_MEMBERSHIP, (char *)&stMreq))
        return;

    /* This is here because we do not have a graceful shutdown.  We will need to design all sockets
     * being closed and all TCP and UDP functions ending before performing any sort of cleanup.
     */
    while (1);

    printf("closing socket\n");
    closesocket(sck);
    WSACleanup();
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void CALLBACK songRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD)
--                     DWORD error: Error that occured during WSARecv()
--                     DWORD bytesTransferred: Amount of bytes read
--                     LPWSAOVERLAPPED overlapped: Pointer to overlapped struct
--
--  RETURNS:       void
--
--  DATE:          March 29, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    Robert Arendac
--
--  NOTES:
--      Completion routine for receiving the song list.  First checks for error or if the
--      socket was closed.  Will then parse each song name into a list.  The client window
--      track list is then updated with this list.
---------------------------------------------------------------------------------------*/
void CALLBACK songRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD)
{
    char *token;
    QStringList songs;

    SocketInformation *si = (SocketInformation *)overlapped;

    // Check for error or close connection request
    if (error != 0 || bytesTransferred == 0)
    {
        if (error)
        {
            fprintf(stderr, "Error: %d\n", error);
        }
        fprintf(stderr, "Closing socket: %d\n", (int)si->socket);
        closesocket(si->socket);
        return;
    }

    //Separate the received data by the newline character and add to songs list
    token = strtok(si->dataBuf.buf, "\n");
    while (token != NULL)
    {
        songs.append(token);
        token = strtok(NULL, "\n");
    }

    //Update songlist on GUI
    clientWind->updateSongs(songs);
}
