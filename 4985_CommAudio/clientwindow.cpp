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
#include "client.h"

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

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void ClientWindow::on_cltConnect_clicked()
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
--      Starts a TCP client and connect to a host on a port, both obtained from the UI.
--      NOTE: Also need to run UDP on separate thread.
---------------------------------------------------------------------------------------*/
void ClientWindow::on_cltConnect_clicked()
{
    runTCPClient(this, ui->cltHostIPEditText->text().toStdString().c_str(), ui->cltPortSpinner->value());
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
