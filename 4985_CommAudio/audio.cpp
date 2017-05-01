/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	clientwindow.cpp
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
QVector<QByteArray> chunks(CHUNKSIZE);
int stopFlag = 0;

int startPos = 0;
int len = OFFSET;

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

    if (prevTrack != currTrack || output->state() == QAudio::IdleState && prevTrack == filePath)
    {
        loadAudioData(currTrack); // load raw data from wav file to a buffer
        loadAudioStream();
        playStream();
    }
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
    {
        prevTrack = currTrack;
        // seek to raw audio data of wav file
        audioFile.seek(AUDIODATA);

        // extract raw audio data
        audioByteData = audioFile.readAll();
        qDebug() << "size: " << audioByteData.size();
        audioFile.close();
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
--      Loads chunks of data into circular buffer (vector)
---------------------------------------------------------------------------------------*/
void loadAudioStream()
{
    QByteArray tempData;

    if(!chunks.isEmpty()) // check if vector has data
    {
        chunks.clear(); // clear the vector of audio chunks
        qDebug() << "cleared";
    }

    // circularbuffer
    for (int i = 0; i < CHUNKSIZE; i++)
    {
        // add audio chunks to vector
        tempData = audioByteData.mid(startPos, len);
        chunks.push_back(tempData);
        tempData.clear();

        startPos += len;
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
{
    QByteArray tmp; // tmp array to hold data chunk
    QBuffer buf(&tmp); // create QBuffer based off of tmp
    buf.open(QIODevice::ReadWrite); // open bffer as QIODevice
    int i = 0; // while loop counter

    while(i < CHUNKSIZE)
    {
        if(stopFlag == 1) // if user clicked stopped. clear buffers
        {
            output->stop();
            audioByteData.clear();
            tmp.clear();
            buf.close();
            stopFlag = 0;
            return;
        }

        // appends chunk of data to play
        tmp.append(chunks[i].data(),chunks[i].size());

        output->start(&buf); // play track
        // event loop for track
        QEventLoop loop;
        QObject::connect(output, SIGNAL(stateChanged(QAudio::State)), &loop, SLOT(quit()));
        do
        {
            loop.exec();
            if(buf.atEnd())
            {
                qDebug() << "End";
                sendAudio(chunks[i].data());
                qDebug() << "size: " << chunks[i].size();
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
--      Stops audio from playing. Set flag to false
---------------------------------------------------------------------------------------*/
void stopAudio()
{
    stopFlag = 1;
}
