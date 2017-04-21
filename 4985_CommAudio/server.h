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
#include "socketinformation.h"
#include "wrappers.h"

//Audio Methods
void pauseAudio();
void playAudio(QString filePath);

//Socket Handling
void removeSocket(SOCKET socket);
void runTCPServer(ServerWindow *sw, int port);
void runUDPServer(ServerWindow *sw, int port);
SOCKADDR_IN serverCreateAddress(int port);
DWORD WINAPI tcpClient(void *arg);


//Request methods
void selectSong(SocketInformation *si);
void sendSongs(SocketInformation *si);
void uploadToClient(SocketInformation *si);
void downloadFromClient(SocketInformation *si);

#endif // SERVER_H
