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
#include <QDebug>
#include <QListWidget>
#include <QDataStream>
#include <QFile>
#include <QBuffer>
#include <QIODevice>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QAudioInput>
#include <QAudioDeviceInfo>
#include <QEventLoop>

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

    pp_counter = 1;


    /**
    QBuffer *buffer;
    QAudioOutput *a;

    QAudioFormat format;
    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(8);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());

    char *data = (char*)malloc(32768 * sizeof(char));

    //generating a sound
    for (int i = 0; i<256; ++i)
    {
       for (int j = 0; j<128; ++j)
       {
           data[i * 128 + j] = (char)j;
       }
    }

    //copying into the buffer
    buffer = new QBuffer;
    buffer->open(QIODevice::ReadWrite);
    buffer->write(data, 32768);
    buffer->seek(0);

    a = new QAudioOutput(format);
    //a->moveToThread(&thr);

    //thr.start();
    //QMetaObject::invokeMethod(a, "start", Q_ARG(QIODevice*, buffer));

    a->start(buffer);
    */
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
    ThreadInfo *ti;

    ti = (ThreadInfo *)malloc(sizeof(ThreadInfo));
    ti->TCPPort = ui->srvTCPPortSpinner->value();
    ti->UDPPort = ui->srvUDPPortSpinner->value();
    ti->sWindow = this;

    CreateThread(NULL, 0, ServerWindow::tcpServerThread, (void *)ti, 0, NULL);
    CreateThread(NULL, 0, ServerWindow::udpServerThread, (void *)ti, 0, NULL);
}

void ServerWindow::on_srvTrackPreviousButton_clicked()
{

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
--
---------------------------------------------------------------------------------------*/
void ServerWindow::on_srvTrackPlayPauseButton_clicked()
{
    // Read in the whole thing
    /**
    char fileType[4];
    qint32 fileSize;
    char waveName[4];
    char fmtName[3];
    qint32 fmtLength;
    short fmtType;
    short numberOfChannels;
    qint32 sampleRate;
    qint32 sampleRateXBitsPerSampleXChanngelsDivEight;
    short bitsPerSampleXChannelsDivEightPointOne;
    short bitsPerSample;
    char dataHeader[4];
    qint32 dataSize;

    if(pp_counter == 0)
    {
        ui->srvTrackPlayPauseButton->setText("Play");
        pp_counter = 1;
    }
    else
    {
        ui->srvTrackPlayPauseButton->setText("Pause");
        pp_counter = 0;
    }

    QString q = ui->musicList->currentItem()->text();
    QString filename = "../Music/" + q;
    qDebug() << filename;

    // Open wave file
    QFile wavFile(filename);
    if (!wavFile.open(QFile::ReadOnly))
    {
     qDebug() << "Failed to open WAV file...";
    }
    else
    {
    qDebug() << "WAV file open";
    }

    // Read in the whole thing
    QByteArray wavFileContent = wavFile.readAll();
    qDebug() << "The size of the WAV file is: " << wavFileContent.size();

    // Create a data stream to analyze the data
    QDataStream analyzeHeaderDS(&wavFileContent,QIODevice::ReadOnly);
    analyzeHeaderDS.setByteOrder(QDataStream::LittleEndian);

    // Now pop off the appropriate data into each header field defined above
    analyzeHeaderDS.readRawData(fileType,4); // "RIFF"
    analyzeHeaderDS >> fileSize; // File Size
    analyzeHeaderDS.readRawData(waveName,4); // "WAVE"
    analyzeHeaderDS.readRawData(fmtName,3); // "fmt"
    analyzeHeaderDS >> fmtLength; // Format length
    analyzeHeaderDS >> fmtType; // Format type
    analyzeHeaderDS >> numberOfChannels; // Number of channels
    analyzeHeaderDS >> sampleRate; // Sample rate
    analyzeHeaderDS >> sampleRateXBitsPerSampleXChanngelsDivEight; // (Sample Rate * BitsPerSample * Channels) / 8
    analyzeHeaderDS >> bitsPerSampleXChannelsDivEightPointOne; // (BitsPerSample * Channels) / 8.1
    analyzeHeaderDS >> bitsPerSample; // Bits per sample
    analyzeHeaderDS.readRawData(dataHeader,4); // "data" header
    analyzeHeaderDS >> dataSize; // Data Size

    // Print the header
    qDebug() << "WAV File Header read:";
    qDebug() << "File Type: " << QString::fromUtf8(fileType);
    qDebug() << "File Size: " << fileSize;
    qDebug() << "WAV Marker: " << QString::fromUtf8(waveName);
    qDebug() << "Format Name: " << QString::fromUtf8(fmtName);
    qDebug() << "Format Length: " << fmtLength;
    qDebug() << "Format Type: " << fmtType;
    qDebug() << "Number of Channels: " << numberOfChannels;
    qDebug() << "Sample Rate: " << sampleRate;
    qDebug() << "Sample Rate * Bits/Sample * Channels / 8: " << sampleRateXBitsPerSampleXChanngelsDivEight;
    qDebug() << "Bits per Sample * Channels / 8.1: " << bitsPerSampleXChannelsDivEightPointOne;
    qDebug() << "Bits per Sample: " << bitsPerSample;
    qDebug() << "Data Header: " << QString::fromUtf8(dataHeader);
    qDebug() << "Data Size: " << dataSize;
    */
    //char *ramBuffer = {};

    //analyzeHeaderDS.readRawData(ramBuffer, (int)dataSize);

/**QStringList list = getSongs();
    QString q = ui->musicList->currentItem()->text();
    qDebug() << q;*/
    /**for (int i = 0; i < list.size(); i++)
    {
        QString tmp = list.at(i);
        qDebug() << tmp;
    }*/

    QFile audio_file("../Music/ChillingMusic.wav");
    if(audio_file.open(QIODevice::ReadOnly)) {
        QAudioFormat format;
        QAudioOutput *output;
        QBuffer *audioBuffer;


        audio_file.seek(44); // skip wav header
        QByteArray audio_data = audio_file.readAll(); // audio data
        audio_file.close();

        //QBuffer* audio_buffer = new QBuffer(&audio_data);
        //audio_buffer->open(QIODevice::ReadWrite);
        //qDebug() << audio_buffer->size();

        //QAudioFormat format;
        format.setSampleSize(16);
        format.setSampleRate(44100);
        format.setChannelCount(2);
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setSampleType(QAudioFormat::UnSignedInt);

        QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
        if (!info.isFormatSupported(format)) {
            qWarning()<<"raw audio format not supported by backend, cannot play audio.";
            return;
        }
        qDebug() << info.deviceName();

        audioBuffer = new QBuffer(&audio_data);
        audioBuffer->open(QIODevice::ReadWrite);
        audioBuffer->seek(0);
        //QAudioOutput* output = new QAudioOutput(format);
        output = new QAudioOutput(format);
        output->start(audioBuffer);
        //output->start(&audio_buffer);
        QEventLoop loop;
        QObject::connect(output, SIGNAL(stateChanged(QAudio::State)), &loop, SLOT(quit()));
        do {
            loop.exec();
        } while(output->state() == QAudio::ActiveState);
    }
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
--      Updates the status label of the client based on string that is passed in
---------------------------------------------------------------------------------------*/
void ServerWindow::updateServerStatus(QString status)
{
    ui->srvStatusLabel->setText(status);
}
