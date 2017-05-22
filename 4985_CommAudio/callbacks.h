#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <WinSock2.h>
#include <Windows.h>

void CALLBACK generalRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
void CALLBACK songRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
void CALLBACK parseRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD);

#endif // CALLBACKS_H
