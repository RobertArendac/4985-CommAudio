#include "client.h"
#include "server.h"
#include <WS2tcpip.h>
#include <QMessageBox>
#include <QFileDialog>

ClientWindow *clientWind;

SOCKET cltSck;      //Connected TCP socket
char filename[SONG_SIZE];

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
--  RETURNS:       void
--
--  DATE:          March 19, 2017
--
--  MODIFIED:      March 30, 2017 - Update client status accordingly ~ AZ
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac, Alex Zielinski
--
--  NOTES:
--      Will connect to a TCP server. Once a connection is established, it will receive
--      a message containing the server song list.
---------------------------------------------------------------------------------------*/
void runTCPClient(ClientWindow *cw, const char *ip, int port)
{
    SocketInformation *si;
    SOCKET sck;                 //Socket to send/receive on
    SOCKADDR_IN addr;           //Address of server
    DWORD recvBytes, flags = 0; //Used for receiving data
    WSAEVENT events[1];         //Event array
    DWORD result;               //Result of waiting for event

    clientWind = cw;    //Init the global ClientWindow

    // Create a TCP socket
    if ((sck = createSocket(SOCK_STREAM, IPPROTO_TCP)) == NULL)
    {
        cw->updateClientStatus("Status: Socket Error");
        return;
    }

    // Check for a valid host
    if (!connectHost(ip))
    {
        cw->updateClientStatus("Status: Host IP Error");
        return;
    }


    // Initialize address info
    memset((char *)&addr, 0, sizeof(SOCKADDR_IN));
    addr = clientCreateAddress(ip, port);

    // Connect to the server
    if (!connectToServer(sck, &addr))
    {
        cw->updateClientStatus("Status: Connection Error");
        return;
    }

    cltSck = sck;

    //Allocate socket information
    si = (SocketInformation *)malloc(sizeof(SocketInformation));

    //Fill in the socket info
    si->socket = sck;
    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));
    si->bytesReceived = 0;
    si->bytesSent = 0;
    si->dataBuf.len = BUF_SIZE;
    si->dataBuf.buf = si->buffer;

    // Receive data, will be the song list
    WSARecv(si->socket, &(si->dataBuf), 1, &recvBytes, &flags, &(si->overlapped), songRoutine);

    //Wait for receive to complete
    events[0] = WSACreateEvent();
    if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
        fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);

    free(si);
    cw->updateClientStatus("Status: Connected");

    /* This is here because we do not have a graceful shutdown.  We will need to design all sockets
     * being closed and all TCP and UDP functions ending before performing any sort of cleanup.
     */
    while(1);

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
--  RETURNS:       void
--
--  DATE:          March 19, 2017
--
--  MODIFIED:      March 30, 2017 - Update client status accordingly ~ AZ
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    Robert Arendac, Alex Zielinski
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
    {
        cw->updateClientStatus("Status: Socket Error");
        return;
    }

    // Check for valid host
    if (!connectHost(ip))
    {
        cw->updateClientStatus("Status: Host IP Error");
        return;
    }

    // Init address info
    memset((char *)&addr, 0, sizeof(SOCKADDR_IN));
    addr = clientCreateAddress(ip, port);

    // Set the reuse addr
    if (!setCltOptions(sck, SO_REUSEADDR, (char *)&flag))
    {
        cw->updateClientStatus("Status: Socket Error");
        return;
    }

    memset((char *)&srvAddr, 0, sizeof(SOCKADDR_IN));
    srvAddr = serverCreateAddress(MCAST_PORT);

    // Bind to multicast group
    if (!bindSocket(sck, &srvAddr))
    {
        cw->updateClientStatus("Status: Socket Error");
        return;
    }

    // Setup multicast interface
    stMreq.imr_multiaddr.s_addr = inet_addr(MCAST_ADDR);
    stMreq.imr_interface.s_addr = INADDR_ANY;

    // Join multicast group
    if (!setServOptions(sck, IP_ADD_MEMBERSHIP, (char *)&stMreq))
    {
        cw->updateClientStatus("Status: Socket Error");
        return;
    }

    /* This is here because we do not have a graceful shutdown.  We will need to design all sockets
     * being closed and all TCP and UDP functions ending before performing any sort of cleanup.
     */
    while (1);

    printf("closing socket\n");
    closesocket(sck);
    WSACleanup();
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void requestSong(const char *song)
--                      const char *song: the song being requested
--
--  RETURNS:       void
--
--  DATE:          April 7, 2017
--
--  DESIGNER:      Matt Goerwell
--
--  PROGRAMMER:    Matt Goerwell
--
--  NOTES:
--      Method that requests a specific song be played by the server. Sends the request type,
--      then sends the name of the song.
---------------------------------------------------------------------------------------*/
void requestSong(const char *song) {
    SocketInformation *si;
    WSAEVENT events[1];
    DWORD result, sendBytes = 0;

    si = (SocketInformation *)malloc(sizeof(SocketInformation));

    si->socket = cltSck;
    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));

    strcpy(si->buffer,"pick");
    si->bytesReceived = 0;
    si->bytesSent = 0;
    si->dataBuf.len = BUF_SIZE;
    si->dataBuf.buf = si->buffer;

    // Send the request type;
    WSASend(si->socket, &(si->dataBuf), 1, &sendBytes, 0, &(si->overlapped), pickRoutine);

    // Wait for the send to complete
    events[0] = WSACreateEvent();
    if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
        fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);

    ResetEvent(events[0]);
    //Reset buffers for next send.
    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));

    //load in song name
    strcpy(si->buffer, song);
    si->bytesReceived = 0;
    si->bytesSent = 0;
    si->dataBuf.len = BUF_SIZE;
    si->dataBuf.buf = si->buffer;

    //send in song name
    WSASend(si->socket, &(si->dataBuf), 1, &sendBytes, 0, &(si->overlapped), pickRoutine);

    // Wait for the send to complete
    if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
        fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void updateClientSongs()
--
--  RETURNS:       void
--
--  DATE:          April 7, 2017
--
--  DESIGNER:      Matt Goerwell
--
--  PROGRAMMER:    Matt Goerwell
--
--  NOTES:
--      Method that requests an update to the song list from the server. Sends the request type
--      then waits to receive the song list.
---------------------------------------------------------------------------------------*/
void updateClientSongs() {
    SocketInformation *si;
    WSAEVENT events[1];
    DWORD result, sendBytes, recvBytes, flags = 0;

    si = (SocketInformation *)malloc(sizeof(SocketInformation));

    si->socket = cltSck;
    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));
    strcpy(si->buffer,"updoot");
    si->bytesReceived = 0;
    si->bytesSent = 0;
    si->dataBuf.len = BUF_SIZE;
    si->dataBuf.buf = si->buffer;

    // Send the request type;
    WSASend(si->socket, &(si->dataBuf), 1, &sendBytes, 0, &(si->overlapped), pickRoutine);

    // Wait for the send to complete
    events[0] = WSACreateEvent();
    if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
        fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);

    ResetEvent(events[0]);

    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));
    si->bytesReceived = 0;
    si->bytesSent = 0;
    si->dataBuf.len = BUF_SIZE;
    si->dataBuf.buf = si->buffer;

    WSARecv(si->socket, &(si->dataBuf), 1, &recvBytes, &flags, &(si->overlapped), songRoutine);
    if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
    fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void downloadSong(const char *song)
--                     const char *song - Song to download
--
--  RETURNS:
--
--  DATE:          April 3, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    Robert Arendac
--
--  NOTES:
--      Temporary download song method.  Will send song name to server and then prepare
--      to download it.  User is first prompted to enter a save path before any downloading begins
---------------------------------------------------------------------------------------*/
void downloadSong(const char *song)
{
    FILE *fp;
    SocketInformation *si;
    WSAEVENT events[1];
    DWORD result, sendBytes, recvBytes, flags = 0;

    si = (SocketInformation *)malloc(sizeof(SocketInformation));

    si->socket = cltSck;
    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));
    //memset(si->filename, 0, sizeof(si->filename));

    // Get the save location
    strcpy(filename, QFileDialog::getSaveFileName(NULL, "Save Audio File", song, "(*.wav) (*.mp3)").toStdString().c_str());

    // Clear the file
    fp = fopen(filename, "w");
    fclose(fp);

    strcpy(si->buffer, song);
    si->bytesReceived = 0;
    si->bytesSent = 0;
    si->dataBuf.len = BUF_SIZE;
    si->dataBuf.buf = si->buffer;

    // Sends the song name
    WSASend(si->socket, &(si->dataBuf), 1, &sendBytes, 0, &(si->overlapped), sendRoutine);

    // Wait for the send to complete
    events[0] = WSACreateEvent();
    if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
        fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);

    ResetEvent(events[0]);

    // Reset buffers in preparation to receive
    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));
    strcpy(si->buffer, song);
    si->bytesReceived = 0;
    si->bytesSent = 0;
    si->dataBuf.len = BUF_SIZE;
    si->dataBuf.buf = si->buffer;

    // Receives song file
    WSARecv(si->socket, &(si->dataBuf), 1, &recvBytes, &flags, &(si->overlapped), downloadRoutine);
    if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
        fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);

    // Notify user download is complete, could probably be refined into something better
    QMessageBox msg(QMessageBox::Information, "Notice:", "Download complete!");
    msg.exec();

    free(si);

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

    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void CALLBACK sendRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD)
--                     DWORD error: Error that occured during WSASend()
--                     DWORD bytesTransferred: Amount of bytes sent
--                     LPWSAOVERLAPPED overlapped: Pointer to overlapped struct
--
--  RETURNS:       void
--
--  DATE:          April 3, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    Robert Arendac
--
--  NOTES:
--      Completion routine for sending.  Simply checks if something went wrong and then
--      clears the buffers.
---------------------------------------------------------------------------------------*/
void CALLBACK sendRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD)
{
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

    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));

}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void CALLBACK downloadRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
--                     DWORD error: Error that occured during WSASend()
--                     DWORD bytesTransferred: Amount of bytes sent
--                     LPWSAOVERLAPPED overlapped: Pointer to overlapped struct
--                     DWORD flags: flags for receiving
--
--  RETURNS:       void
--
--  DATE:          April 3, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    Robert Arendac
--
--  NOTES:
--      Completion routine for downloading a song.  Receives each packet and appends it
--      to the save file
---------------------------------------------------------------------------------------*/
void CALLBACK downloadRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
{
    SocketInformation *si = (SocketInformation *)overlapped;
    DWORD recvBytes;
    FILE *fp;

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

    // Check for end of transmission
    if (strcmp(si->dataBuf.buf, "COMPLETE") == 0)
        return;

    //Write chunk to file
    fp = fopen(filename, "a+b");
    fwrite(si->dataBuf.buf, 1, bytesTransferred, fp);
    fclose(fp);

    // Reset buffers for next receive
    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));

    si->dataBuf.len = BUF_SIZE;
    si->dataBuf.buf = si->buffer;

    WSARecv(si->socket, &(si->dataBuf), 1, &recvBytes, &flags, &(si->overlapped), downloadRoutine);
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void CALLBACK pickRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD)
--                     DWORD error: Error that occured during WSARecv()
--                     DWORD bytesTransferred: Amount of bytes read
--                     LPWSAOVERLAPPED overlapped: Pointer to overlapped struct
--
--  RETURNS:       void
--
--  DATE:          April 3, 2017
--
--  DESIGNER:      Matt Goerwell
--
--  PROGRAMMER:    Matt Goerwell
--
--  NOTES:
--      Completion routine for requesting a specific song be played. Checks for error or if the
--      socket was closed.
---------------------------------------------------------------------------------------*/
void CALLBACK pickRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD)
{

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
}
