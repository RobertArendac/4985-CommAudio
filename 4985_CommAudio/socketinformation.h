#ifndef SOCKETINFORMATION_H
#define SOCKETINFORMATION_H
#include <WinSock2.h>
#define BUF_SIZE 5000   //size of the buffer parameter for socketInformation

// Hold all the information associated with a socket
typedef struct SocketInformation
{
    OVERLAPPED overlapped;
    char buffer[BUF_SIZE];
    //char filename[BUF_SIZE];
    WSABUF dataBuf;
    SOCKET socket;
    DWORD bytesSent;
    DWORD bytesReceived;
} SocketInformation;

#endif // SOCKETINFORMATION_H
