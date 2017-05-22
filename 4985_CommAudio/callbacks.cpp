#include <stdio.h>
#include <stdlib.h>
#include "wrappers.h"
#include "server.h"
#include "callbacks.h"
#include "socketinformation.h"

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
--      General completion routine that checks for errors or close connections, and updates
--      amount of bytes sent/received.
---------------------------------------------------------------------------------------*/
void CALLBACK generalRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD)
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

    si->bytesReceived = si->bytesSent = bytesTransferred;

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
    si->cWindow->updateSongs(songs);

    resetBuffers(si);
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
