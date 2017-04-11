#ifndef AUDIO_H
#define AUDIO_H

#include <QFile>
#include <QBuffer>
#include <QIODevice>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QAudioInput>
#include <QAudioDeviceInfo>
#include <QEventLoop>
#include <QList>
#include <QDebug>
#include <QVector>
#include <QQueue>

#define CHANNELCOUNT 2
#define SAMPLERATE 44100
#define SAMPLESIZE 16
#define AUDIODATA 44
#define OFFSET 100000

bool audioPlaying();
void loopEvent();
void app(int i);
void resetPrevTrack();
void initAudioOutput();
void play(QString filePath);
void loadAudioData(QString filePath);
void loadAudioStream();
void playStream();
void stopAudio();

#endif // AUDIO_H
