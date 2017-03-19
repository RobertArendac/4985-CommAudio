/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	serverwindow.cpp
--
--	FUNCTIONS:
--
--	DATE:			March 19, 2017
--
--	DESIGNERS: Alex Zielinski
--
--	PROGRAMMERS: Alex Zielinski
--
--	NOTES:
--      This is the server window which will act as a radio station. Clients can connect
--      to the server and listen to the audio that the server is streaming. The server
--      relays the audio to all connected clients and relays microphone input from a client
--      to all connected clients
---------------------------------------------------------------------------------------*/

#include "serverwindow.h"
#include "ui_serverwindow.h"

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

void ServerWindow::on_srvStartStopButton_clicked()
{

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
