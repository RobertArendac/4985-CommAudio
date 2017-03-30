/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	serverwindow.cpp
--
--	FUNCTIONS:
--
--	DATE:			March 19, 2017
--
--	DESIGNERS:      Alex Zielinski
--
--	PROGRAMMERS:    Alex Zielinski
--
--	NOTES:
--      This is the server window which will act as a radio station. Clients can connect
--      to the server and listen to the audio that the server is streaming. The server
--      relays the audio to all connected clients and relays microphone input from a client
--      to all connected clients
---------------------------------------------------------------------------------------*/

#include "serverwindow.h"
#include "ui_serverwindow.h"
#include "server.h"
#include <QDir>

#define CLIENT_SIZE 32

ServerWindow::ServerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ServerWindow)
{
    ui->setupUi(this);
    ui->srvStatusLabel->setText("Status: OFF");
    // Start a winsock session
    if (!startWinsock())
        return;

    createSongList();
}

ServerWindow::~ServerWindow()
{
    delete ui;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void ServerWindow::on_srvStartStopButton_clicked()
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
--      Starts the server on button click.  NOTE: Still need to setup UDP server for audio
--      streaming, should be on separate thread. Also, hardcoding port number for now
---------------------------------------------------------------------------------------*/
void ServerWindow::on_srvStartStopButton_clicked()
{
    CreateThread(NULL, 0, ServerWindow::tcpServerThread, this, 0, NULL);
    CreateThread(NULL, 0, ServerWindow::udpServerThread, this, 0, NULL);
}

void ServerWindow::on_srvTrackPreviousButton_clicked()
{

}

void ServerWindow::on_srvTrackRWButton_clicked()
{

}

void ServerWindow::on_srvTrackPlayPauseButton_clicked()
{

}

void ServerWindow::on_srvTrackFFButton_clicked()
{

}

void ServerWindow::on_srvTrackNextButton_clicked()
{

}

void ServerWindow::on_srvShuffleRadioButton_clicked()
{

}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     DWORD WINAPI ServerWindow::tcpServerThread(void *arg)
--                     void *arg: ServerWindow to pass to the TCP server
--
--  RETURNS:       Thread exit condition
--
--  DATE:          March 25, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    Robert Arendac
--
--  NOTES:
--      Simple thread that starts the UDP server.
---------------------------------------------------------------------------------------*/
DWORD WINAPI ServerWindow::udpServerThread(void *arg)
{
    ServerWindow *sw = (ServerWindow *)arg;

    runUDPServer(sw, 7000);

    return 0;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     DWORD WINAPI ServerWindow::tcpServerThread(void *arg)
--                     void *arg: ServerWindow to pass to the TCP server
--
--  RETURNS:       Thread exit condition
--
--  DATE:          March 25, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    Robert Arendac
--
--  NOTES:
--      Simple thread that starts the TCP server.
---------------------------------------------------------------------------------------*/
DWORD WINAPI ServerWindow::tcpServerThread(void *arg)
{
    ServerWindow *sw = (ServerWindow *)arg;

    runTCPServer(sw, 8980);

    return 0;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     QStringList ServerWindow::getSongs()
--
--  RETURNS:       A string list of all songs
--
--  DATE:          March 26, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    Robert Arendac
--
--  NOTES:
--      Static method that gets the songs in the Music folder and returns them
---------------------------------------------------------------------------------------*/
QStringList ServerWindow::getSongs()
{
    QDir directory("../Music");
    QStringList songs = directory.entryList();

    return songs;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void ServerWindow::createSongList()
--
--  RETURNS:       void
--
--  DATE:          March 26, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    Robert Arendac
--
--  NOTES:
--      Gets the songs in the Music folder and adds them to the GUI
---------------------------------------------------------------------------------------*/
void ServerWindow::createSongList()
{
    QStringList list = getSongs();
    for (auto song : list)
    {
        ui->musicList->addItem(song);
    }
}

/********************************************************
 *  Function:       void ServerWindow::updateClients(const char *client)
 *                      const char *client: Client IP address
 *
 *  Programmer:     Robert Arendac
 *
 *  Created:        Mar 25 2017
 *
 *  Modified:
 *
 *  Desc:
 *      Adds a connected client to the servers client list
 *******************************************************/
void ServerWindow::updateClients(const char *client)
{
    char newClient[CLIENT_SIZE];
    sprintf(newClient, "%s", client);
    ui->srvClientListWidgest->addItem(newClient);
}

void ServerWindow::on_srvSelectPlaylistButton_clicked()
{

}


/*--------------------------------------------------------------------------------------
--  INTERFACE:     ServerWindow::updateServerStatus(int status)
--
--  RETURNS:       void
--
--  DATE:          March 30, 2017
--
--  DESIGNER:      Alex Zielinski
--
--  PROGRAMMER:    Alex Zielinski
--
--  NOTES:
--      Updates the status label of the client based on the return value returned by
--      the functions that create the TCP and UDP sockets
---------------------------------------------------------------------------------------*/
void ServerWindow::updateServerStatus(QString status)
{
    ui->srvStatusLabel->setText(status);
}
