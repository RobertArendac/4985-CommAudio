#ifndef SERVER_H
#define SERVER_H

#include <WinSock2.h>
#include <Windows.h>
#include <QFile>
#include <QBuffer>
#include <QIODevice>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QAudioInput>
#include <QAudioDeviceInfo>
#include <QEventLoop>
#include "wrappers.h"

#define CHANNELCOUNT 2
#define SAMPLERATE 44100
#define SAMPLESIZE 16
#define AUDIODATA 44

bool audioPlaying();
void resetPrevSong();
void playAudio(QString &filePath);
void initAudioOutput();
void pauseAudio();
void stopAudio();
void runTCPServer(ServerWindow *sw, int port);
void runUDPServer(ServerWindow *sw, int port);
SOCKADDR_IN serverCreateAddress(int port);
DWORD WINAPI tcpClient(void *arg);
void CALLBACK clientRoutine(DWORD error, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK newRoutine(DWORD error, DWORD Transfered, LPWSAOVERLAPPED overlapped, DWORD);
void sendSongs(SocketInformation *si);

#endif // SERVER_H
