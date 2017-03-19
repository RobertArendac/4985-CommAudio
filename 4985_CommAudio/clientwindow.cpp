/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	clientwindow.cpp
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
--      This is the client window where users can connect to a server to listen to songs
--      (like a radio station). Users can also use a microphone connected to their machine
--      to talk to other clients connected to the server.
---------------------------------------------------------------------------------------*/

#include "clientwindow.h"
#include "ui_clientwindow.h"

ClientWindow::ClientWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ClientWindow)
{
    ui->setupUi(this);
}

ClientWindow::~ClientWindow()
{
    delete ui;
}

void ClientWindow::on_pushButton_clicked()
{

}

void ClientWindow::on_cltSelectAllButton_clicked()
{

}

void ClientWindow::on_cltUpdateButton_clicked()
{

}

void ClientWindow::on_cltUploadButton_clicked()
{

}

void ClientWindow::on_cltPlaySelectedTrackButton_clicked()
{

}

void ClientWindow::on_cltDownloadSelectedTrackButton_clicked()
{

}
