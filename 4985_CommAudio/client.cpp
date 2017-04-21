#include "client.h"
#include "server.h"
#include "callbacks.h"
#include <WS2tcpip.h>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include "socketinformation.h"
ClientWindow *clientWind;

SOCKET cltSck;      //Connected TCP socket

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
    DWORD flags = 0;            //Used for receiving data

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

    // Set global socket
    cltSck = sck;

    //Allocate socket information
    si = (SocketInformation *)malloc(sizeof(SocketInformation));

    //Fill in the socket info
    si->socket = sck;
    si->cWindow = clientWind;
    resetBuffers(si);

    // Receive data, will be the song list
    if (!recvData(si, &flags, songRoutine))
        exit(1);

    free(si);
    cw->updateClientStatus("Status: Connected");
    cw->enableButtons();
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
    SocketInformation *si;
    DWORD result;
    WSAEVENT events[1];         //Event array

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

    //Allocate socket information
    si = (SocketInformation *)malloc(sizeof(SocketInformation));

    //Fill in the socket info
    si->socket = sck;
    resetBuffers(si);

    //Testing UDP works, use as template for actually doing something useful
    /*
    WSASendTo(si->socket, &(si->dataBuf), 1, NULL, 0, (SOCKADDR *)&addr, sizeof(SOCKADDR_IN), &(si->overlapped), clientRoutine);

    //Wait for receive to complete
    events[0] = WSACreateEvent();
    if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
        fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);

    */

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

    si = (SocketInformation *)malloc(sizeof(SocketInformation));

    si->socket = cltSck;
    resetBuffers(si);
    strcpy(si->buffer,"pick");

    // Send the request type;
    if (!sendData(si, pickRoutine))
        exit(1);

    //Reset buffers for next send.
    resetBuffers(si);

    //load in song name
    strcpy(si->buffer, song);

    //send in song name
    if (!sendData(si, pickRoutine))
        exit(1);

    free(si);
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
    DWORD flags = 0;

    si = (SocketInformation *)malloc(sizeof(SocketInformation));

    si->socket = cltSck;
    si->cWindow = clientWind;
    resetBuffers(si);
    strcpy(si->buffer,"update");

    // Send the request type;
    if (!sendData(si, pickRoutine))
        exit(1);

    resetBuffers(si);

    if (!recvData(si, &flags, songRoutine))
        exit(1);

    free(si);
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
--  MODIFIED:      April 11, file size is sent over now instead of using an end-of-transmission indicator
--
--  NOTES:
--      Will send song name to server and then prepare to download it.
--      User is first prompted to enter a save path before any downloading begins
---------------------------------------------------------------------------------------*/
void downloadSong(const char *song)
{
    FILE *fp;
    SocketInformation *si;
    DWORD flags = 0;
    char filename[SONG_SIZE];

    si = (SocketInformation *)malloc(sizeof(SocketInformation));

    si->socket = cltSck;
    resetBuffers(si);

    // Get the save location
    memset(filename, 0, sizeof(filename));
    strcpy(filename, QFileDialog::getSaveFileName(NULL, "Save Audio File", song, "(*.wav) (*.mp3)").toStdString().c_str());

    // Check if user pressed cancel
    if (filename[0] == '\0')
        return;

    // Clear the file
    fp = fopen(filename, "w");
    fclose(fp);

    resetBuffers(si);
    strcpy(si->buffer, "dl");

    // Sends the request type
    if (!sendData(si, sendRoutine))
        exit(1);

    // Reset buffers in preparation to send again
    resetBuffers(si);
    strcpy(si->buffer, song);

    // Sends the song name
    if (!sendData(si, sendRoutine))
        exit(1);

    // Reset buffers in preparation to receive
    resetBuffers(si);

    // Receives song size
    if (!recvData(si, &flags, pickRoutine))
        exit(1);

    int size = atoi(si->dataBuf.buf);
    int totalBytes = 0;

    // Reset buffers for next receive
    resetBuffers(si);

    fp = fopen(filename, "a+b");
    while (totalBytes < size)
    {
        // Receives song file
        if (!recvData(si, &flags, pickRoutine))
            exit(1);

        //Write chunk to file
        fwrite(si->dataBuf.buf, 1, si->bytesReceived, fp);

        totalBytes += si->bytesReceived;

        // Reset buffers for next receive
        resetBuffers(si);
    }
    fclose(fp);

    // Notify user download is complete, could probably be refined into something better
    QMessageBox msg(QMessageBox::Information, "Notice:", "Download complete!");
    msg.exec();

    free(si);
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void uploadSong(QString song)
--                     QString song: Filepath of song to upload
--
--  RETURNS:       void
--
--  DATE:          April 8, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac
--
--  NOTES:
--      Uploads a song to the server.  Will first send request type.  Then parses the song
--      name and sends that.  Once server knows song name, client begins transferring file.
---------------------------------------------------------------------------------------*/
void uploadSong(QString song)
{
    FILE *fp;
    SocketInformation *si;
    DWORD flags = 0;
    QFile f(song);
    QFileInfo fi(f.fileName());
    char filename[SONG_SIZE];

    si = (SocketInformation *)malloc(sizeof(SocketInformation));

    si->socket = cltSck;
    si->cWindow = clientWind;
    resetBuffers(si);

    // Get the file location
    memset(filename, 0, sizeof(filename));
    strcpy(filename, song.toStdString().c_str());

    // Check if user pressed cancel
    if (filename[0] == '\0')
        return;

    // Prepare upload request
    resetBuffers(si);
    strcpy(si->buffer, "ul");

    // Sends the request
    if (!sendData(si, sendRoutine))
        exit(1);

    // Reset buffers in preparation to send song name
    resetBuffers(si);
    strcpy(si->buffer, fi.fileName().toStdString().c_str());

    // Sends the song name
    if (!sendData(si, sendRoutine))
        exit(1);

    // Reset buffers in preparation to upload song
    resetBuffers(si);

    int sz;
    fp = fopen(filename, "r+b");
    fseek(fp, 0, SEEK_END);
    sz = ftell(fp); // Size of the file
    rewind(fp);
    int loops;

    // Calculate how many loops are needed and size of last packet
    loops = sz / BUF_SIZE + (sz % BUF_SIZE != 0);
    int lastSend = sz - ((loops - 1) * BUF_SIZE);

    resetBuffers(si);

    sprintf(si->buffer, "%d", sz);

    // Send file size
    if (!sendData(si, clientRoutine))
        exit(1);

    for (int i = 0; i < loops; i++)
    {
        resetBuffers(si);

        // Last iteration, read smaller packet
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

        // Send packet
        if (!sendData(si, clientRoutine))
            exit(1);
    }

    fclose(fp);

    // Reset buffers to receive updated song list
    resetBuffers(si);

    // Receive data, will be the song list
    if (!recvData(si, &flags, songRoutine))
        exit(1);

    free(si);
}
