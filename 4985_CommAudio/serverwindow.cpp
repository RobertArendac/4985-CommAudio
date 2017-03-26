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

ServerWindow::ServerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ServerWindow)
{
    ui->setupUi(this);
}

ServerWindow::~ServerWindow()
{
    delete ui;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void ServerWindow::on_srvStartStopButton_clicked()
--
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
    runTCPServer(this, 5150);
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

void ServerWindow::on_srvPlaySelectedTrackButton_clicked()
{

}

DWORD WINAPI ServerWindow::udpServerThread(void *arg) {
    ServerWindow *sw = (ServerWindow *)arg;

    runUDPServer(sw, 5150);

    return 0;
}
