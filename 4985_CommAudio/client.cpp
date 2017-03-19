#include "client.h"

SOCKADDR_IN clientCreateAddress(char *host, int port) {
    SOCKADDR_IN addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons(port);

    return addr;
}

void runTCPClient(ClientWindow *cw, char *ip, int port) {
    SOCKET sck;
    SOCKADDR_IN addr;

    if (!startWinsock())
        return;

    if ((sck = createSocket(SOCK_STREAM, IPPROTO_TCP)) == NULL)
        return;

    if (!connectHost(ip))
        return;

    memset((char *)&addr, 0, sizeof(SOCKADDR_IN));
    addr = clientCreateAddress(ip, port);

    if (!connectToServer(sck, &addr)) {
        return;
    }

    // Do stuff

    printf("closing socket");
    closesocket(sck);
    WSACleanup();
}

void runUDPClient(ClientWindow *cw, char *ip, int port) {
    SOCKET sck;
    SOCKADDR_IN addr;

    if (!startWinsock())
        return;

    if ((sck = createSocket(SOCK_STREAM, IPPROTO_TCP)) == NULL)
        return;

    if (!connectHost(ip))
        return;

    memset((char *)&addr, 0, sizeof(SOCKADDR_IN));
    addr = clientCreateAddress(ip, port);


    // Do stuff

    printf("closing socket");
    closesocket(sck);
    WSACleanup();
}
