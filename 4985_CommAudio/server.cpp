#include "server.h"
#include "client.h"
#include <map>
#include <QDebug>
#include <WS2tcpip.h>

std::map<SOCKET, std::string> clientMap;
char filepath[SONG_SIZE];
ServerWindow *servWin;

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
--  RETURNS:       void
--
--  DATE:          March 19, 2017
--
--  MODIFIED:      March 29, 2017 - Made function return an int ~ AZ
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    Robert Arendac, Alex Zielinski
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
    {
        sw->updateServerStatus("Status: Socket Error");
        return;
    }

    // Initialize address info
    addr = serverCreateAddress(port);

    // Bind the listening socket
    if (!bindSocket(listenSocket, &addr))
    {
        sw->updateServerStatus("Status: Socket Error");
        return;
    }

    // Set socket to listen for connection
    if (!listenConnection(listenSocket))
    {
        sw->updateServerStatus("Status: Socket Error");
        return;
    }

    servWin = sw;
    sw->updateServerStatus("Status: ON");
    // Accept incoming connections and put each client on thread
    while (1)
    {
        if (!acceptingSocket(&acceptSocket, listenSocket, (SOCKADDR *)&clientAddr))
        {
            return;
        }

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
    SOCKET *clientSck = (SOCKET *)arg;      //Client socket
    DWORD flags = 0;  //Bytes to be sent
    SocketInformation *si;                  //Struct holding socket info
    WSAEVENT events[1];                     //Array of events (just one)
    DWORD result;                           //Result of waiting for event

    // Fill out the socket info
    si = (SocketInformation *)malloc(sizeof(SocketInformation));
    si->socket = *clientSck;

    sendSongs(si);

    //Client request Parse loop
    events[0] = WSACreateEvent();
    while (true)
    {
        ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
        memset(si->buffer, 0, sizeof(si->buffer));
        si->bytesReceived = 0;
        si->bytesSent = 0;
        si->dataBuf.len = BUF_SIZE;
        si->dataBuf.buf = si->buffer;
        WSARecv(si->socket, &(si->dataBuf), 1, NULL, &flags, &(si->overlapped), parseRoutine);
        if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
            fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);
        ResetEvent(events[0]);

        if (strcmp("pick", si->buffer) == 0)
        {
            selectSong(si);
        }
        else if (strcmp("dl", si->buffer) == 0)
        {
            uploadToClient(si);
        }
        else if (strcmp("ul", si->buffer) == 0)
        {
            downloadFromClient(si);
        }
        else if (strcmp("update", si->buffer) == 0)
        {
           sendSongs(si);
        }
    }

    return 0;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void sendSongs(SocketInformation *si)
--                     SocketInformation *si: Struct holding all information needed for sending
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
--      Sends a list of songs to the client
---------------------------------------------------------------------------------------*/
void sendSongs(SocketInformation *si)
{
    char music[SONG_SIZE];              //C-string of songs
    WSAEVENT events[1];                 //Array of events (just one)
    std::string songlist;               //String of all songs
    DWORD result;

    // Build the song list
    QStringList songs = ServerWindow::getSongs();
    for (auto song : songs)
    {
        songlist += song.toStdString() + "\n";
    }

    // Copy the song list to a c-string, can't send std::string
    strcpy(music, songlist.c_str());

    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));
    strcpy(si->buffer, music);
    si->bytesReceived = 0;
    si->bytesSent = 0;
    si->dataBuf.len = BUF_SIZE;
    si->dataBuf.buf = si->buffer;

    // Send the song list
    WSASend(si->socket, &(si->dataBuf), 1, NULL, 0, &(si->overlapped), clientRoutine);

    // Wait for the send to complete
    events[0] = WSACreateEvent();
    if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
        fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);
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
--  INTERFACE:     void CALLBACK parseRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD)
--                     DWORD error: error that occured during WSARecv()
--                     DWORD bytesTransferred: the number of bytes received
--                     LPWSAOVERLAPPED overlapped: pointer to overlapped struct
--                     Other arg unused
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
--      Completion routine for receiving a client request.  Checks for errors, then parses the
--      request type made and proceeds to the appropriate method.
---------------------------------------------------------------------------------------*/
void CALLBACK parseRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD)
{
    SocketInformation *si = (SocketInformation *)overlapped;
    // Check for error or close connection request
    if (error != 0 || bytesTransferred == 0)
    {
        if (error)
        {
            fprintf(stderr, "Error: %d\n", error);
        }
        removeSocket(si->socket);
        return;
    }
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void selectSong(SocketInformation *si)
--                     SocketInformation *si: Pointer to a valid Socket Info Struct
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
--      Method that plays a specific song for the user, in response to a request.
---------------------------------------------------------------------------------------*/
void selectSong(SocketInformation *si)
{
    DWORD result, flags = 0;
    WSAEVENT events[1];
    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));
    si->bytesReceived = 0;
    si->bytesSent = 0;
    si->dataBuf.len = BUF_SIZE;
    si->dataBuf.buf = si->buffer;

    // Receive the song
    WSARecv(si->socket, &(si->dataBuf), 1, NULL, &flags, &(si->overlapped), clientRoutine);

    // Wait for the receive to complete
    events[0] = WSACreateEvent();
    if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
        fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);

    //temporary until I know how song selection works. replace this with playing the song, once ALEX has it ready.
    fprintf(stdout,"Song name: %s\n",si->buffer);
}


/*--------------------------------------------------------------------------------------
--  INTERFACE:     void runTCPServer(ServerWindow *sw, int port)
--                     ServerWindow *sw: GUI to update
--                     int port: port to bind to
--
--  RETURNS:       void
--
--  DATE:          March 19, 2017
--
--  MODIFIED:      March 28, 2017 - Added multicasting capabilities
--                 March 29, 2017 - Made function return an int ~ AZ
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac, Alex Zielinski
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
    SocketInformation *si;
    DWORD result, flags = 0;
    WSAEVENT events[1];

    // Init address info
    addr = serverCreateAddress(port);

    // Create a socket for incomming data
    if ((acceptSocket = createSocket(SOCK_DGRAM, IPPROTO_UDP)) == NULL)
    {
        sw->updateServerStatus("Status: Socket Error");
        return;
    }

    // bind the socket
    if (!bindSocket(acceptSocket, &addr))
    {
        sw->updateServerStatus("Status: Socket Error");
        return;
    }

    // Multicast interface
    stMreq.imr_multiaddr.s_addr = inet_addr(MCAST_ADDR);
    stMreq.imr_interface.s_addr = INADDR_ANY;

    // Join multicast group, specify time-to-live, and disable loop
    if (!setServOptions(acceptSocket, IP_ADD_MEMBERSHIP, (char *)&stMreq))
    {
        sw->updateServerStatus("Status: Socket Error");
        return;
    }

    if (!setServOptions(acceptSocket, IP_MULTICAST_TTL, (char *)&ttl))
    {
        sw->updateServerStatus("Status: Socket Error");
        return;
    }

    if (!setServOptions(acceptSocket, IP_MULTICAST_LOOP, (char *)&flag))
    {
        sw->updateServerStatus("Status: Socket Error");
        return;
    }

    cltDest = clientCreateAddress(MCAST_ADDR, MCAST_PORT);

    //Allocate socket information
    si = (SocketInformation *)malloc(sizeof(SocketInformation));

    //Fill in the socket info
    si->socket = acceptSocket;
    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));
    si->bytesReceived = 0;
    si->bytesSent = 0;
    si->dataBuf.len = BUF_SIZE;
    si->dataBuf.buf = si->buffer;

    while (1)
    {
        //Testing UDP works, use as template for actually doing something useful
        /*
        WSARecvFrom(si->socket, &(si->dataBuf), 1, NULL, &flags, NULL, NULL, &(si->overlapped), newRoutine);

        //Wait for receive to complete
        events[0] = WSACreateEvent();
        if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
            fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);

        qDebug() << si->dataBuf.buf << endl;
        */

    }
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void downloadFromClient(SocketInformation *si)
--                     SocketInformation *si: Pointer to struct containing socket info
--
--  RETURNS:       void
--
--  DATE:          April 9, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac, Alex Zielinski
--
--  NOTES:
--      Downloads a song from the client and adds it to the music list
---------------------------------------------------------------------------------------*/
void downloadFromClient(SocketInformation *si)
{
    WSAEVENT events[1];
    DWORD result, flags = 0;

    // Get the save location
    memset(filepath, 0, sizeof(filepath));
    strcpy(filepath, "../Music/");

    // Reset buffers in preparation to receive
    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));
    si->bytesReceived = 0;
    si->bytesSent = 0;
    si->dataBuf.len = BUF_SIZE;
    si->dataBuf.buf = si->buffer;

    // Receives the song name
    WSARecv(si->socket, &(si->dataBuf), 1, NULL, &flags, &(si->overlapped), clientRoutine);

    events[0] = WSACreateEvent();
    if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
        fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);

    ResetEvent(events[0]);

    // Add the songname to the filepath
    strcat(filepath, si->dataBuf.buf);

    // Clear the file and make it empty
    FILE *fp;
    fp = fopen(filepath, "w");
    fclose(fp);

    // Reset buffers in preparation to receive
    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));
    si->bytesReceived = 0;
    si->bytesSent = 0;
    si->dataBuf.len = BUF_SIZE;
    si->dataBuf.buf = si->buffer;

    // Receives the song size
    WSARecv(si->socket, &(si->dataBuf), 1, NULL, &flags, &(si->overlapped), clientRoutine);
    if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
        fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);

    ResetEvent(events[0]);

    int size = atoi(si->dataBuf.buf);
    int totalBytes = 0;

    // Reset buffers for next receive
    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));

    si->dataBuf.len = BUF_SIZE;
    si->dataBuf.buf = si->buffer;

    fp = fopen(filepath, "a+b");
    if (fp == NULL)
        qDebug() << errno;
    while (totalBytes < size)
    {
        // Receives song file
        WSARecv(si->socket, &(si->dataBuf), 1, NULL, &flags, &(si->overlapped), pickRoutine);
        if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
            fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);
        ResetEvent(events[0]);

        //Write chunk to file

        fwrite(si->dataBuf.buf, 1, si->bytesReceived, fp);

        totalBytes += si->bytesReceived;

        // Reset buffers for next receive
        ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
        memset(si->buffer, 0, sizeof(si->buffer));

        si->dataBuf.len = BUF_SIZE;
        si->dataBuf.buf = si->buffer;
    }
    fclose(fp);

    // Notify user download is complete, could probably be refined into something better
    qDebug() << "Download complete!" << endl;

    // Send an updated song list
    sendSongs(si);

    // Update the server UI with the new song
    servWin->createSongList();
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void uploadToClient(SocketInformation *si)
--                     SocketInformation *si: Pointer to struct containing socket info
--
--  RETURNS:       void
--
--  DATE:          April 9, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac, Alex Zielinski
--
--  NOTES:
--      Uploads a song to the client
---------------------------------------------------------------------------------------*/
void uploadToClient(SocketInformation *si)
{
    DWORD result, flags = 0;
    WSAEVENT events[1];
    char musicPath[SONG_SIZE] = "../Music/";

    // Receive the name of the song to upload
    WSARecv(si->socket, &(si->dataBuf), 1, NULL, &flags, &(si->overlapped), clientRoutine);

    //Wait for receive to complete
    events[0] = WSACreateEvent();
    if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
        fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);

    ResetEvent(events[0]);

    // Add the file name to the filepath
    strcat(musicPath, si->dataBuf.buf);

    FILE *fp;
    int sz;
    fp = fopen(musicPath, "r+b");
    fseek(fp, 0, SEEK_END);
    sz = ftell(fp); // Size of the file
    rewind(fp);
    int loops;

    // Calculate how many loops are needed and size of last packet
    loops = sz / BUF_SIZE + (sz % BUF_SIZE != 0);
    int lastSend = sz - ((loops - 1) * BUF_SIZE);

    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));
    si->bytesReceived = 0;
    si->bytesSent = 0;

    sprintf(si->buffer, "%d", sz);

    WSASend(si->socket, &(si->dataBuf), 1, NULL, 0, &(si->overlapped), clientRoutine);

    // Wait for the send to complete
    if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
        fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);

    ResetEvent(events[0]);

    for (int i = 0; i < loops; i++)
    {
        ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
        memset(si->buffer, 0, sizeof(si->buffer));
        si->bytesReceived = 0;
        si->bytesSent = 0;

        // On the last iteration, send smaller packet
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

        // Send a single packet
        WSASend(si->socket, &(si->dataBuf), 1, NULL, 0, &(si->overlapped), clientRoutine);

        // Wait for the send to complete
        if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
            fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);

        ResetEvent(events[0]);
    }

    // Transfer finished, close file
    fclose(fp);
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void CALLBACK srvDownloadRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
--                     DWORD error: Error that occured during WSASend()
--                     DWORD bytesTransferred: Amount of bytes sent
--                     LPWSAOVERLAPPED overlapped: Pointer to overlapped struct
--                     DWORD flags: flags for receiving
--
--  RETURNS:       void
--
--  DATE:          April 8, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    Robert Arendac
--
--  NOTES:
--      Completion routine for downloading a song.  Receives each packet and appends it
--      to the save file
---------------------------------------------------------------------------------------*/
void CALLBACK srvDownloadRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
{
    SocketInformation *si = (SocketInformation *)overlapped;
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
    fp = fopen(filepath, "a+b");
    if (fp == NULL)
        qDebug() << errno;
    fwrite(si->dataBuf.buf, 1, bytesTransferred, fp);
    fclose(fp);

    // Reset buffers for next receive
    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));

    si->dataBuf.len = BUF_SIZE;
    si->dataBuf.buf = si->buffer;

    // Receive another packet
    WSARecv(si->socket, &(si->dataBuf), 1, NULL, &flags, &(si->overlapped), srvDownloadRoutine);
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void removeSocket(SOCKET socket)
--                     SOCKET socket: the socket(client) that is being removed.
--
--  RETURNS:       void
--
--  DATE:          April 10, 2017
--
--  DESIGNER:      Matt Goerwell
--
--  PROGRAMMER:    Matt Goerwell
--
--  NOTES:
--      Removes a socket(client) from the server's list of clients.
---------------------------------------------------------------------------------------*/
void removeSocket(SOCKET socket)
{
    qDebug() << "Closing socket: " << (int)socket;
    const char *client = clientMap.find(socket)->second.c_str();
    servWin->removeClient(client);
    clientMap.erase(socket);
    closesocket(socket);
}
