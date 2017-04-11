/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	clientwindow.cpp
--
--	FUNCTIONS:      void initAudioOutput();
--                  void play(QString filePath);
--                  void loadAudioData(QString filePath);
--                  void loadAudioStream();
--                  void playStream();
--
--
--	DATE:			April 8, 2017
--
--	DESIGNERS:      Alex Zielinski
--
--	PROGRAMMERS:    Alex Zielinski
--
--	NOTES:
--      This file contains all functions that deal with audio processing and playing
---------------------------------------------------------------------------------------*/

#include "audio.h"
#include "server.h"
#include <thread>

QAudioOutput *output;
QBuffer audioBuffer;
QByteArray audioByteData;
QString prevTrack;
QString currTrack;
QVector<QByteArray> chunks(10);
int currPos = OFFSET;
int startPos = 0;

int a = 0;
int b = 500000;

/*--------------------------------------------------------------------------------------
--  INTERFACE:     bool audioPlaying()
--
--  RETURNS:       returns true if audio is playing. Otherwise false;
--
--  DATE:          April 6, 2017
--
--  DESIGNER:      Alex Zielinski
--
--  PROGRAMMER:    Alex Zielinski
--
--  NOTES:
--      checks if audio is currently playing and returns appropriate bool value
---------------------------------------------------------------------------------------*/
bool audioPlaying()
{
    return (output->state() == QAudio::ActiveState);
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void resetPrevTrack()
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
--      Resets the string of the previous song to be empty
---------------------------------------------------------------------------------------*/
void resetPrevTrack()
{
    prevTrack.clear();
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void initAudioOutput()
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
--      initializes audio format and audio output
---------------------------------------------------------------------------------------*/
void initAudioOutput()
{
    QAudioFormat format;

    // set audio playback formatting
    format.setSampleSize(SAMPLESIZE);
    format.setSampleRate(SAMPLERATE);
    format.setChannelCount(CHANNELCOUNT);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    // setup default output device
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(format))
    {
        qDebug() << "raw audio format not supported by backend, cannot play audio.";
        return;
    }

    // initialize output
    output = new QAudioOutput(format);
    audioBuffer.open(QIODevice::ReadWrite);
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void play(QString filePath)
--                      QString filePath:  file path of audio file
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
--      used to call all functions responsible for processing and playing audio
---------------------------------------------------------------------------------------*/
void play(QString filePath)
{
    currTrack = filePath; // set current track to what user selected
    loadAudioData(currTrack); // load raw data from wav file to a buffer
    loadAudioStream();
    playStream();
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void loadAudioData(QString filePath)
--                      QString filePath:  file path of audio file
--
--  RETURNS:       void
--
--  DATE:          April 8, 2017
--
--  DESIGNER:      Alex Zielinski
--
--  PROGRAMMER:    Alex Zielinski
--
--  NOTES:
--     Copies raw audio PCM data from wav file to a buffer
---------------------------------------------------------------------------------------*/
void loadAudioData(QString filePath)
{
    // create file handle for audio file
    QFile audioFile(filePath);
    // open audio file
    if (audioFile.open(QIODevice::ReadOnly))
    {   // check if user selected a different track
        if (prevTrack != currTrack || output->state() == QAudio::IdleState && prevTrack == filePath)
        {
            prevTrack = currTrack;
            // seek to raw audio data of wav file
            audioFile.seek(AUDIODATA);

            // extract raw audio data
            audioByteData = audioFile.readAll();
            qDebug() << "size: " << audioByteData.size();
        }
    }
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void loadAudioStream()
--
--  RETURNS:       void
--
--  DATE:          April 8, 2017
--
--  DESIGNER:      Alex Zielinski
--
--  PROGRAMMER:    Alex Zielinski
--
--  NOTES:
--      Plays audio from stream of data
---------------------------------------------------------------------------------------*/
void loadAudioStream()
{
    QByteArray tempData;

    if(!chunks.isEmpty())
    {
        chunks.clear();
        qDebug() << "cleared";
    }

    for (int i = 0; i < 10; i++)
    {
        tempData = audioByteData.mid(a, b);
        chunks.push_back(tempData);
        tempData.clear();

        a += b;
    }
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void playStream()
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
--      Plays audio from stream of data
---------------------------------------------------------------------------------------*/
void playStream()
{   output->setNotifyInterval(1680000);
    QByteArray tmp;
    QBuffer buf(&tmp);
    buf.open(QIODevice::ReadWrite);
    int i = 0;

    qDebug() << "start";

    //int x = 0;
    while(i < 10)
    {
        if(chunks[i].size() == 0)
        {
            qDebug() << "song done";
            return;
        }

        tmp.append(chunks[i].data(),chunks[i].size());
        //audioBuffer.seek(x);
        //x += chunks[i].size();

        output->start(&buf); // play track
        // event loop for track
        QEventLoop loop;
        QObject::connect(output, SIGNAL(stateChanged(QAudio::State)), &loop, SLOT(quit()));
        do
        {
            loop.exec();
            if(audioBuffer.atEnd())
            {
               qDebug() << "end " << i;
            }
        } while(output->state() == QAudio::ActiveState);

        if(i == 9)
        {
            qDebug() << "circular buffer";
            i = 0;
            loadAudioStream();
        }
        else
        {
            i++;
        }
    }
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void stopAudio()
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
--      Stops audio from playing
---------------------------------------------------------------------------------------*/
void stopAudio()
{
    // check if audio is playing
    if (output->state() == QAudio::ActiveState)
    {
        output->stop(); // stop the audio
        output->reset();
        audioBuffer.close();
    }
}
