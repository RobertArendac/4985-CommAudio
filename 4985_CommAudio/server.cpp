#include "server.h"

SOCKADDR_IN serverCreateAddress(int port) {
    SOCKADDR_IN addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    return addr;
}

void runTCPServer(ServerWindow *sw, int port) {
    SOCKET listenSocket, acceptSocket;
    SOCKADDR_IN addr, clientAddr;

    if (!startWinsock())
        return;

    if ((listenSocket = createSocket(SOCK_STREAM, IPPROTO_TCP)) == NULL)
        return;

    addr = serverCreateAddress(port);

    if (!bindSocket(listenSocket, &addr))
        return;

    if (!listenConnection(listenSocket))
        return;

    while (1) {
        if (!acceptingSocket(&acceptSocket, listenSocket, (SOCKADDR *)&clientAddr))
            return;

        // Just for confirmation, take out when we have UI client list set up
        printf("Client %s connected\n", inet_ntoa(clientAddr.sin_addr));

        CreateThread(NULL, 0, tcpClient, &acceptSocket, 0, NULL);
    }

}

DWORD WINAPI tcpClient(void *arg) {
    SOCKET *clientSck = (SOCKET *)arg;

    // Do stuff

    return 0;
}

void runUDP(ServerWindow *sw, int port) {
    SOCKADDR_IN addr;

    if (!startWinsock())
        return;

    addr = serverCreateAddress(port);

    if ((acceptSocket = createSocket(SOCK_DGRAM, IPPROTO_UDP)) == NULL)
        return;

    if (!bindSocket(acceptSocket, &addr))
        return;

    while (1) {
        // Do stuff
    }
}
