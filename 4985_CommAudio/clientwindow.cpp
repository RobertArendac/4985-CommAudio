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
#include "threadinfo.h"
#include <QFile>
#include <QFileDialog>

ClientWindow::ClientWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ClientWindow)
{

    ui->setupUi(this);
    ui->cltStatusLabel->setText("Status: Not connected");
    // Start a winsock session
    if (!startWinsock())
        return;
}

ClientWindow::~ClientWindow()
{
    delete ui;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void ClientWindow::on_cltConnect_clicked()
--
--  RETURNS:       void
--
--  DATE:          March 19, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac
--
--  NOTES:
--      Starts a TCP/UDP client and connect to a host on a port, both obtained from the UI.
---------------------------------------------------------------------------------------*/
void ClientWindow::on_cltConnect_clicked()
{
    ThreadInfo *ti;

    ti = (ThreadInfo *)malloc(sizeof(ThreadInfo));
    strcpy(ti->cltIP, ui->cltHostIPEditText->text().toStdString().c_str());
    ti->TCPPort = ui->cltTCPPortSpinner->value();
    ti->UDPPort = ui->cltUDPPortSpinner->value();
    ti->cWindow = this;

    runTCPClient(this, ti->cltIP, ti->TCPPort);

    //CreateThread(NULL, 0, ClientWindow::udpClientThread, (void *)ti, 0, NULL);

}


/*--------------------------------------------------------------------------------------
--  INTERFACE:     void ClientWindow::on_cltUpdateButton_clicked()
--
--  RETURNS:       void
--
--  DATE:          April 4, 2017
--
--  DESIGNER:      Matt Goerwell
--
--  PROGRAMMER:    Matt Goerwell
--
--  NOTES:
--      Requests an update to the song list from the server.
---------------------------------------------------------------------------------------*/
void ClientWindow::on_cltUpdateButton_clicked()
{
    updateClientSongs();
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void ClientWindow::on_cltUploadButton_clicked()
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
--      Requests an upload to the server and begins transferring a new song.
---------------------------------------------------------------------------------------*/
void ClientWindow::on_cltUploadButton_clicked()
{
    uploadSong(QFileDialog::getOpenFileName(this,"Select File",qApp->applicationDirPath(),"Songs (*.wav *.mp3)"));
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void ClientWindow::on_cltPlaySelectedTrackButton_clicked()
--
--  RETURNS:       void
--
--  DATE:          April 4, 2017
--
--  DESIGNER:      Matt Goerwell
--
--  PROGRAMMER:    Matt Goerwell
--
--  NOTES:
--      Requests that a specific song be played by the server.
---------------------------------------------------------------------------------------*/
void ClientWindow::on_cltPlaySelectedTrackButton_clicked()
{
    if (ui->songList->currentItem() != NULL )
    {
        const QString& song = ui->songList->currentItem()->text();
        requestSong(song.toStdString().c_str());
    }

}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void ClientWindow::on_cltDownloadSelectedTrackButton_clicked()
--
--  RETURNS:
--
--  DATE:          April 3, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac
--
--  NOTES:
--      Downloads selected song.  NOTE: Must have a song selected
---------------------------------------------------------------------------------------*/
void ClientWindow::on_cltDownloadSelectedTrackButton_clicked()
{
    downloadSong(ui->songList->currentItem()->text().toStdString().c_str());
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     DWORD WINAPI ClientWindow::udpClientThread(void *arg)
--                     void *arg: ClientWindow to pass to the TCP client
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
--      Simple thread that starts the TCP client.
---------------------------------------------------------------------------------------*/
DWORD WINAPI ClientWindow::tcpClientThread(void *arg) {
    ThreadInfo *ti = (ThreadInfo *)arg;

    runTCPClient(ti->cWindow, ti->cltIP, ti->TCPPort);

    return 0;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     DWORD WINAPI ClientWindow::tcpServerThread(void *arg)
--                     void *arg: ClientWindow to pass to the udp client
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
--      Simple thread that starts the UDP client.
---------------------------------------------------------------------------------------*/
DWORD WINAPI ClientWindow::udpClientThread(void *arg) {
    ThreadInfo *ti = (ThreadInfo *)arg;

    runUDPClient(ti->cWindow, ti->cltIP, ti->UDPPort);

    return 0;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void ClientWindow::updateSongs(QStringList songs)
--                     QStringList songs: String list of songs
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
--      Adds each song in the song list to the GUI.
---------------------------------------------------------------------------------------*/
void ClientWindow::updateSongs(QStringList songs)
{
    ui->songList->clear();
    for (auto song : songs)
    {
        ui->songList->addItem(song);
    }

    // set first item to be currently selected
    ui->songList->setCurrentItem(ui->songList->item(0));
}


/*--------------------------------------------------------------------------------------
--  INTERFACE:     void keyPressEvent(QKeyEvent* e);
--                     QKeyEvent* e: the key that was pressed
--
--  RETURNS:       void
--
--  DATE:          March 29, 2017
--
--  DESIGNER:      Alex Zielinski
--
--  PROGRAMMER:    Alex Zielinski
--
--  NOTES:
--      Event handler for when the letter K is pressed (also held)
---------------------   ------------------------------------------------------------------*/
void ClientWindow::keyPressEvent(QKeyEvent* e)
{
    if(!e->isAutoRepeat())
    {
        if(e->key() == Qt::Key_K)
        {
            ui->cltMicLabel->setText("Status: ON");
        }
    }
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void clientWindow::keyReleaseEvent(QKeyEvent* e);
--                     QKeyEvent* e: the key that was released
--
--  RETURNS:       void
--
--  DATE:          March 29, 2017
--
--  DESIGNER:      Alex Zielinski
--
--  PROGRAMMER:    Alex Zielinski
--
--  NOTES:
--      Event handler for when the letter K is released
---------------------------------------------------------------------------------------*/
void ClientWindow::keyReleaseEvent(QKeyEvent* e)
{
    if(!e->isAutoRepeat())
    {
        if(e->key() == Qt::Key_K)
        {
            ui->cltMicLabel->setText("Status: OFF");
        }
    }
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void clientWindow::updateClientStatus(int status)
--                     int status: return value of a function to determine status of client
--
--  RETURNS:       void
--
--  DATE:          March 29, 2017
--
--  DESIGNER:      Alex Zielinski
--
--  PROGRAMMER:    Alex Zielinski
--
--  NOTES:
--      Updates the status label of the client based on string that is passed in
---------------------------------------------------------------------------------------*/
void ClientWindow::updateClientStatus(QString status)
{
    ui->cltStatusLabel->setText(status);
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void clientWindow::enableButtons()
--
--  RETURNS:       void
--
--  DATE:          April 11, 2017
--
--  DESIGNER:      Matt Goerwell
--
--  PROGRAMMER:    Matt Goerwell
--
--  NOTES:
--      Enables the use of ui buttons after a connection is confirmed.
---------------------------------------------------------------------------------------*/
void ClientWindow::enableButtons()
{
    ui->cltUpdateButton->setEnabled(true);
    ui->cltUploadButton->setEnabled(true);
    ui->cltPlaySelectedTrackButton->setEnabled(true);
    ui->cltDownloadSelectedTrackButton->setEnabled(true);
}
