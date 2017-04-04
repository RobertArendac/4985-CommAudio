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
SOCKADDR_IN serverCreateAddress(int port)
{
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
--  PROGRAMMER:    Robert Arendac
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
    while (1)
    {
        if (!acceptingSocket(&acceptSocket, listenSocket, (SOCKADDR *)&clientAddr))
            return;

        sw->updateClients(inet_ntoa(clientAddr.sin_addr));
        clientMap.insert(std::pair<SOCKET, std::string>(acceptSocket, inet_ntoa(clientAddr.sin_addr)));
        printf("Client %s connected\n", inet_ntoa(clientAddr.sin_addr));

        CreateThread(NULL, 0, tcpClient, &acceptSocket, 0, NULL);
    }

}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     DWORD WINAPI tcpClient(void *arg)
--                     void *arg: Socket bound to specific client
--
--  RETURNS:       Thread exit information
--
--  DATE:          March 25, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    Robert Arendac
--
--  NOTES:
--      Thread for handling each client.  Will build a song list and send it out to the
--      client.
---------------------------------------------------------------------------------------*/
DWORD WINAPI tcpClient(void *arg)
{
    SOCKET *clientSck = (SOCKET *)arg;  //Client socket
    std::string songlist;               //String of all songs
    DWORD sendBytes, recvBytes, flags = 0;                    //Bytes to be sent
    SocketInformation *si;              //Struct holding socket info
    char music[SONG_SIZE];              //C-string of songs
    WSAEVENT events[1];                 //Array of events (just one)
    DWORD result;                       //Result of waiting for events

    // Build the song list
    QStringList songs = ServerWindow::getSongs();
    for (auto song : songs)
    {
        songlist += song.toStdString() + "\n";
    }

    // Copy the song list to a c-string, can't send std::string
    strcpy(music, songlist.c_str());

    // Fill out the socket info
    si = (SocketInformation *)malloc(sizeof(SocketInformation));
    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));
    strcpy(si->buffer, music);
    si->socket = *clientSck;
    si->bytesReceived = 0;
    si->bytesSent = 0;
    si->dataBuf.len = BUF_SIZE;
    si->dataBuf.buf = si->buffer;

    // Send the song list
    WSASend(si->socket, &(si->dataBuf), 1, &sendBytes, 0, &(si->overlapped), clientRoutine);

    // Wait for the send to complete
    events[0] = WSACreateEvent();
    if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
        fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);

    ResetEvent(events[0]);

   // ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));

    si->dataBuf.len = BUF_SIZE;
    si->dataBuf.buf = si->buffer;

    WSARecv(si->socket, &(si->dataBuf), 1, &recvBytes, &flags, &(si->overlapped), clientRoutine);

    //Wait for receive to complete
    events[0] = WSACreateEvent();
    if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
        fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);

    ResetEvent(events[0]);

    FILE *fp;
    int sz;
    fp = fopen("../Music/Queen - I Want to Break Free.mp3", "r+b");
    fseek(fp, 0, SEEK_END);
    sz = ftell(fp);
    rewind(fp);
    int loops;

    loops = sz / BUF_SIZE + (sz % BUF_SIZE != 0);
    int lastSend = sz - ((loops - 1) * BUF_SIZE);

    for (int i = 0; i < loops; i++)
    {
        ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
        memset(si->buffer, 0, sizeof(si->buffer));
        si->bytesReceived = 0;
        si->bytesSent = 0;

        if (i == loops - 1)
        {
            fread(si->buffer, 1, lastSend, fp);
            si->dataBuf.len = lastSend;
            si->dataBuf.buf = si->buffer;
        }
        else
        {
            fread(si->buffer, 1, BUF_SIZE, fp);

            si->dataBuf.len = BUF_SIZE;
            si->dataBuf.buf = si->buffer;
        }

        WSASend(si->socket, &(si->dataBuf), 1, &sendBytes, 0, &(si->overlapped), clientRoutine);

        // Wait for the send to complete
        if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
            fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);

        ResetEvent(events[0]);
    }

    fclose(fp);

    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));

    si->dataBuf.buf = si->buffer;
    si->dataBuf.len = BUF_SIZE;

    sprintf(si->dataBuf.buf, "%c", 4);
    WSASend(si->socket, &(si->dataBuf), 1, &sendBytes, 0, &(si->overlapped), clientRoutine);

    // Wait for the send to complete
    if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
        fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);

    ResetEvent(events[0]);

    return 0;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void CALLBACK clientRoutine(DWORD error, DWORD, LPWSAOVERLAPPED, DWORD)
--                     DWORD error: error that occured during WSASend()
--                     Other args unused
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
--      Completion routine for sending song list.  In this case, we just want to check
--      for error.
---------------------------------------------------------------------------------------*/
void CALLBACK clientRoutine(DWORD error, DWORD, LPWSAOVERLAPPED, DWORD)
{
    if (error)
    {
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
--  MODIFIED:      March 28, 2017 - Added multicasting capabilities
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
    SOCKADDR_IN addr, cltDest;  //Addresses to receive from and send to
    SOCKET acceptSocket;        //Connect to send/receive on
    struct ip_mreq stMreq;      //Struct for multicasting
    u_long ttl = MCAST_TTL;     //Time to live
    int flag = 0;               //False flag

    // Init address info
    addr = serverCreateAddress(port);

    // Create a socket for incomming data
    if ((acceptSocket = createSocket(SOCK_DGRAM, IPPROTO_UDP)) == NULL)
        return;

    // bind the socket
    if (!bindSocket(acceptSocket, &addr))
        return;

    // Multicast interface
    stMreq.imr_multiaddr.s_addr = inet_addr(MCAST_ADDR);
    stMreq.imr_interface.s_addr = INADDR_ANY;

    // Join multicast group, specify time-to-live, and disable loop
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
