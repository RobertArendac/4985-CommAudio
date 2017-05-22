/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	server.cpp
--
--	DATE:			March 19, 2017
--
--	DESIGNERS:      Robert Arendac
--
--	PROGRAMMERS:    Robert Arendac, Alex Zielinski, Matt Goerwell
--
--	NOTES:
--      Contians network related functions that a server will need to perform
---------------------------------------------------------------------------------------*/

#include "server.h"
#include "client.h"
#include "callbacks.h"
#include "audio.h"
#include <map>
#include <QDebug>
#include <WS2tcpip.h>

std::map<SOCKET, std::string> clientMap;
ServerWindow *servWin;
SOCKADDR_IN multiDest;
SOCKET audioSock;


/*--------------------------------------------------------------------------------------
--  INTERFACE:     void sendAudio(const char *data)
--                     const char *data: Audio data to send
--
--  RETURNS:       void
--
--  DATE:          April 11, 2017
--
--  DESIGNER:      Alex Zielinski
--
--  PROGRAMMER:    Alex Zielinski
--
--  NOTES:
--      Function to send audio data passed in via parameter through a UDP socket
---------------------------------------------------------------------------------------*/
void sendAudio(const char *data)
{
    // send data and error check
    if (sendto(audioSock, data, OFFSET, 0, (struct sockaddr*)&multiDest, sizeof(multiDest)) < 0)
    {
        qDebug() << "sendto failed: " << WSAGetLastError();
    }
    else
    {
        qDebug() << "Sent Audio Data";
    }
}

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
    memset((char *)&addr, 0, sizeof(SOCKADDR_IN));

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
--  MODIFIED:      March 29, 2017 - Made function return int ~ AZ
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
            sw->updateServerStatus("Status: Could not accept connection");
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
    DWORD flags = 0;                        //Receive flags
    SocketInformation *si;                  //Struct holding socket info

    // Fill out the socket info
    si = (SocketInformation *)malloc(sizeof(SocketInformation));
    si->socket = *clientSck;

    sendSongs(si);

    //Client request Parse loop
    while (true)
    {
        resetBuffers(si);

        if (!recvData(si, &flags, parseRoutine))
            exit(1);
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
    std::string songlist;               //String of all songs

    // Build the song list
    QStringList songs = ServerWindow::getSongs();
    for (auto song : songs)
    {
        songlist += song.toStdString() + "\n";
    }

    // Copy the song list to a c-string, can't send std::string
    strcpy(music, songlist.c_str());

    resetBuffers(si);
    strcpy(si->buffer, music);

    if (!sendData(si, generalRoutine))
        exit(1);

    resetBuffers(si);
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
    DWORD flags = 0;

    resetBuffers(si);

    // Receive the song
    if (!recvData(si, &flags, generalRoutine))
        exit(1);

    QString fileName = si->buffer;
    QString audioFilePath = "../Music/" + fileName;
    play(audioFilePath);
}


/*--------------------------------------------------------------------------------------
--  INTERFACE:     void runUDPServer(ServerWindow *sw, int port)
--                     ServerWindow *sw: GUI to update
--                     int port: port to bind to
--
--  RETURNS:       void
--
--  DATE:          March 19, 2017
--
--  MODIFIED:      March 28, 2017 - Added multicasting capabilities
--                 March 29, 2017 - Made function return an int ~ AZ
--                 April 11, 2017 - Re-structered UDP socket to fix bug ~ RA, AZ
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    Robert Arendac, Alex Zielinski
--
--  NOTES:
--      Starts up a UDP server.  Will be responsible for streaming audio.  Also sets up
--      multicasting
---------------------------------------------------------------------------------------*/
void runUDPServer(ServerWindow *sw, int port)
{
    SOCKADDR_IN addr, destAddr;
    SOCKET udpSck;
    int flag = 0;
    struct ip_mreq stMreq;
    u_long ttl = MCAST_TTL;

    // create socket and error check
    if ((udpSck = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        qDebug() << "failed to create socket " << WSAGetLastError();
        return;
    }

    // set socket addr struct
    addr.sin_family      = AF_INET; // ip_v4
    addr.sin_addr.s_addr = htonl(INADDR_ANY); /* any interface */
    addr.sin_port        = 0;                 /* any port */

    // bind socket and error check
    if (bind(udpSck, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        qDebug() << "Bind failed: " << WSAGetLastError();
        return;
    }

    // set multicast interface
    stMreq.imr_multiaddr.s_addr = inet_addr(MCAST_ADDR);
    stMreq.imr_interface.s_addr = INADDR_ANY;

    /* set socket multicast options and error check */
    if (setsockopt(udpSck, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&stMreq, sizeof(stMreq)) == SOCKET_ERROR)
    {
        qDebug() << "setsockopt failed: " << WSAGetLastError();
        return;
    }

    if (setsockopt(udpSck, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(ttl)) == SOCKET_ERROR)
    {
        qDebug() << "setsockopt failed: " << WSAGetLastError();
        return;
    }

    if (setsockopt(udpSck, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&flag, sizeof(flag)) == SOCKET_ERROR)
    {
        qDebug() << "setsockopt failed: " << WSAGetLastError();
        return;
    }


    // set destination addr struct
    destAddr.sin_family      = AF_INET;
    destAddr.sin_addr.s_addr = inet_addr(MCAST_ADDR); /* any interface */
    destAddr.sin_port        = htons(MCAST_PORT);                 /* any port */

    audioSock = udpSck;
    multiDest = destAddr;
    qDebug() << "Ready to send data";
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
--  PROGRAMMER:    Robert Arendac
--
--  MODIFIED:      April 11, file size is sent over now instead of using an end-of-transmission indicator
--
--  NOTES:
--      Downloads a song from the client and adds it to the music list
---------------------------------------------------------------------------------------*/
void downloadFromClient(SocketInformation *si)
{
    DWORD flags = 0;
    char filepath[SONG_SIZE];

    // Get the save location
    memset(filepath, 0, sizeof(filepath));
    strcpy(filepath, "../Music/");

    // Reset buffers in preparation to receive
    resetBuffers(si);

    // Receives the song name
    if (!recvData(si, &flags, generalRoutine))
        exit(1);

    // Add the songname to the filepath
    strcat(filepath, si->dataBuf.buf);

    // Clear the file and make it empty
    FILE *fp;
    fp = fopen(filepath, "w");
    fclose(fp);

    // Reset buffers in preparation to receive
    resetBuffers(si);

    // Receives the song size
    if (!recvData(si, &flags, generalRoutine))
        exit(1);

    int size = atoi(si->dataBuf.buf);
    int totalBytes = 0;

    // Reset buffers for next receive
    resetBuffers(si);

    // Open file for appending
    fp = fopen(filepath, "a+b");
    if (fp == NULL)
        qDebug() << errno;

    // Read while the total file size hasn't been transferred
    while (totalBytes < size)
    {
        // Receives song file
        if (!recvData(si, &flags, generalRoutine))
            exit(1);

        //Write chunk to file
        fwrite(si->dataBuf.buf, 1, si->bytesReceived, fp);

        // Increment bytes received
        totalBytes += si->bytesReceived;

        // Reset buffers for next receive
        resetBuffers(si);
    }

    // Transmission done, close file
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
--  PROGRAMMER:    RobertArendac
--
--  NOTES:
--      Uploads a song to the client
---------------------------------------------------------------------------------------*/
void uploadToClient(SocketInformation *si)
{
    DWORD flags = 0;
    char musicPath[SONG_SIZE] = "../Music/";

    // Receive the name of the song to upload
    if (!recvData(si, &flags, generalRoutine))
        exit(1);

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

    resetBuffers(si);

    sprintf(si->buffer, "%d", sz);

    // Send file size
    if (!sendData(si, generalRoutine))
        exit(1);

    for (int i = 0; i < loops; i++)
    {
        resetBuffers(si);

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
        if (!sendData(si, generalRoutine))
            exit(1);
    }

    // Transfer finished, close file
    fclose(fp);
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
