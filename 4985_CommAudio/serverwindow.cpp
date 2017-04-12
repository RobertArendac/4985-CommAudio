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
#include "audio.h"
#include <QDir>
#include <QListWidget>
#include <QDataStream>

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
    initAudioOutput();
}

ServerWindow::~ServerWindow()
{
    delete ui;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void ServerWindow::on_srvStartStopButton_clicked()
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
--      Starts the server on button click.  NOTE: Still need to setup UDP server for audio
--      streaming, should be on separate thread. Also, hardcoding port number for now
---------------------------------------------------------------------------------------*/
void ServerWindow::on_srvStartStopButton_clicked()
{
    ThreadInfo *ti;

    ti = (ThreadInfo *)malloc(sizeof(ThreadInfo));
    ti->TCPPort = ui->srvTCPPortSpinner->value();
    ti->UDPPort = ui->srvUDPPortSpinner->value();
    ti->sWindow = this;

    CreateThread(NULL, 0, ServerWindow::tcpServerThread, (void *)ti, 0, NULL);
    CreateThread(NULL, 0, ServerWindow::udpServerThread, (void *)ti, 0, NULL);
}

void ServerWindow::on_srvTrackRWButton_clicked()
{

}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void ServerWindow::on_srvTrackPlayPauseButton_clicked()
--
--  RETURNS:       void
--
--  DATE:          April 1, 2017
--
--  DESIGNER:      Alex Zielinski
--
--  PROGRAMMER:    Alex Zielinski
--
--  NOTES:
--      Onclick method for play button. Plays selected song
---------------------------------------------------------------------------------------*/
void ServerWindow::on_srvTrackPlayButton_clicked()
{
    // get track name that is currently selected from widget list
    QString audioFilePath = "../Music/" + ui->musicList->currentItem()->text();
    play(audioFilePath);
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
--      Updates the status label of the client based on string that is passed in
---------------------------------------------------------------------------------------*/
void ServerWindow::updateServerStatus(QString status)
{
    ui->srvStatusLabel->setText(status);
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void ServerWindow::on_srvTrackStopButton_clicked()
--
--  RETURNS:       void
--
--  DATE:          April 4, 2017
--
--  DESIGNER:      Alex Zielinski
--
--  PROGRAMMER:    Alex Zielinski
--
--  NOTES:
--      Stops playing audio if audio is playing
---------------------------------------------------------------------------------------*/
void ServerWindow::on_srvTrackStopButton_clicked()
{
    stopAudio();
    resetPrevTrack();
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void ServerWindow::on_srvTrackPauseButton_clicked()
--
--  RETURNS:       void
--
--  DATE:          April 4, 2017
--
--  DESIGNER:      Alex Zielinski
--
--  PROGRAMMER:    Alex Zielinski
--
--  NOTES:
--      Pause audio if audio is playing
---------------------------------------------------------------------------------------*/
void ServerWindow::on_srvTrackPauseButton_clicked()
{
    char test[OFFSET] = "asdfdsklfjkldsjfklsadfjkdsljfklas;fjskdl";
    sendAudio(test);
}

void ServerWindow::on_srvTrackFFButton_clicked()
{

}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void ServerWindow::playNextTrack()
--
--  RETURNS:       void
--
--  DATE:          April 6, 2017
--
--  DESIGNER:      Alex Zielinski
--
--  PROGRAMMER:    Alex Zielinski
--
--  NOTES:
--      Function containing actual logic to go to next track. It will play the next
--      track if audio is currently playing. If no audio is playing then it simply sets
--      the current position of the musicList to be the next track.
---------------------------------------------------------------------------------------*/
void ServerWindow::playNextTrack()
{
    // check if current element is last element
    if (ui->musicList->currentRow() == ui->musicList->count() - 1)
    {
        // set current row to first row
        ui->musicList->setCurrentRow(0);

        // get track name that is currently selected from widget list
        QString audioFilePath = "../Music/" + ui->musicList->currentItem()->text();

        // check if audio is playing
        if(audioPlaying())
        {   // keep playing audio
            play(audioFilePath);
        }
    }
    else // current element is not last element
    {
        // set current row to be next row
        ui->musicList->setCurrentRow(ui->musicList->currentRow() + 1);

        // get track name that is currently selected from widget list
        QString audioFilePath = "../Music/" + ui->musicList->currentItem()->text();

        // check if audio is playing
        if(audioPlaying())
        {   // keep playing audio
            play(audioFilePath);
        }
    }
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void ServerWindow::playPrevTrack()
--
--  RETURNS:       void
--
--  DATE:          April 6, 2017
--
--  DESIGNER:      Alex Zielinski
--
--  PROGRAMMER:    Alex Zielinski
--
--  NOTES:
--      Function containing actual logic to go to previous track. It will play the next
--      track if audio is currently playing. If no audio is playing then it simply sets
--      the current position of the musicList to be the previous track.
---------------------------------------------------------------------------------------*/
void ServerWindow::playPrevTrack()
{
    // check if current element is first element
    if (ui->musicList->currentRow() == 0)
    {
        // set current row to last row
        ui->musicList->setCurrentRow(ui->musicList->count() - 1);

        // get track name that is currently selected from widget list
        QString audioFilePath = "../Music/" + ui->musicList->currentItem()->text();

        // check if audio is playing
        if(audioPlaying())
        {   // keep playing audio
            play(audioFilePath);
        }
    }
    else // current element is not first element
    {
        // set current row be 1 row behind
        ui->musicList->setCurrentRow(ui->musicList->currentRow() - 1);

        // get track name that is currently selected from widget list
        QString audioFilePath = "../Music/" + ui->musicList->currentItem()->text();

        // check if audio is playing
        if(audioPlaying())
        {   // keep playing audio
            play(audioFilePath);
        }
    }
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void ServerWindow::on_srvTrackNextButton_clicked()
--
--  RETURNS:       void
--
--  DATE:          April 6, 2017
--
--  DESIGNER:      Alex Zielinski
--
--  PROGRAMMER:    Alex Zielinski
--
--  NOTES:
--      onClick function for clicking on the next track button
---------------------------------------------------------------------------------------*/
void ServerWindow::on_srvTrackNextButton_clicked()
{
    playNextTrack();
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void ServerWindow::on_srvTrackPreviousButton_clicked()
--
--  RETURNS:       void
--
--  DATE:          April 6, 2017
--
--  DESIGNER:      Alex Zielinski
--
--  PROGRAMMER:    Alex Zielinski
--
--  NOTES:
--      onClick function for clicking on the previous track button
---------------------------------------------------------------------------------------*/
void ServerWindow::on_srvTrackPreviousButton_clicked()
{
    playPrevTrack();
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
    ThreadInfo *ti = (ThreadInfo *)arg;

    runUDPServer(ti->sWindow, ti->UDPPort);

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
    ThreadInfo *ti = (ThreadInfo *)arg;

    runTCPServer(ti->sWindow, ti->TCPPort);

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
    songs.removeOne(".");
    songs.removeOne("..");

    return songs;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void ServerWindow::createSongList()
--
--  RETURNS:       void
--
--  DATE:          March 26, 2017
--
--  MODIFIED       April 1 - Set first item of list to be selected by default ~ AZ
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    Robert Arendac, Alex Zielinski
--
--  NOTES:
--      Gets the songs in the Music folder and adds them to the GUI
---------------------------------------------------------------------------------------*/
void ServerWindow::createSongList()
{
    ui->musicList->clear();
    QStringList list = getSongs();
    for (auto song : list)
    {
        ui->musicList->addItem(song);
    }
    // set first item to be currently selected
    ui->musicList->setCurrentItem(ui->musicList->item(0));
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

/********************************************************
 *  Function:       void ServerWindow::removeClient(const char *client)
 *                      const char *client: Client IP address
 *
 *  Programmer:     Alex Zielinski
 *
 *  Created:        Mar 20 2017
 *
 *  Modified:       Matt Goerwell ~ April 10 2017
 *
 *  Desc:
 *      removes a client from the servers client list
 *******************************************************/
void ServerWindow::removeClient(const char *client)
{
    QListWidgetItem *item;
    char itemText[CLIENT_SIZE]; // hold string content of list item

    // iterate through the client list
    for (int row = 0; row < ui->srvClientListWidgest->count(); row++)
    {
        item = ui->srvClientListWidgest->item(row); // get current item
        sprintf(itemText, item->text().toStdString().c_str()); // copys the items string content

        // if list item is matches the client string the delete
        if (strcmp(itemText, client) == 0)
        {  // remove current client row from list
           ui->srvClientListWidgest->takeItem(row);
        }
    }
}

void ServerWindow::on_srvSelectPlaylistButton_clicked()
{

}
