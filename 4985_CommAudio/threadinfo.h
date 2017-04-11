#ifndef THREADINFO_H
#define THREADINFO_H
#define IP_SIZE 16

typedef struct {
    char cltIP[IP_SIZE];
    int TCPPort;
    int UDPPort;
    ServerWindow *sWindow;
    ClientWindow *cWindow;
} ThreadInfo;

#endif // THREADINFO_H
