#include "server.h"
#include <map>

std::map<SOCKET, std::string> clientMap;

/*--------------------------------------------------------------------------------------
--  INTERFACE:     SOCKADDR_IN serverCreateAddress(int port)
--                     int port: port to bind to
--
--  RETURNS:       Struct containing all addressing information
--
--  DATE:          March 19, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac
--
--  NOTES:
--      Fills an address struct.  IP is any, port is passed in, used for TCP.
---------------------------------------------------------------------------------------*/
SOCKADDR_IN serverCreateAddress(int port) {
    SOCKADDR_IN addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    return addr;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void runTCPServer(ServerWindow *sw, int port)
--                     ServerWindow *sw: GUI to update
--                     int port: port to bind to
--
--  RETURNS:       Struct containing all addressing information
--
--  DATE:          March 19, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac
--
--  NOTES:
--      Starts up a TCP server.  Each new client will have its own dedicated thread to
--      service it.
---------------------------------------------------------------------------------------*/
void runTCPServer(ServerWindow *sw, int port)
{
    SOCKET listenSocket, acceptSocket;
    SOCKADDR_IN addr, clientAddr;

    // Create socket for listening
    if ((listenSocket = createSocket(SOCK_STREAM, IPPROTO_TCP)) == NULL)
        return;

    // Initialize address info
    addr = serverCreateAddress(port);

    // Bind the listening socket
    if (!bindSocket(listenSocket, &addr))
        return;

    // Set socket to listen for connection
    if (!listenConnection(listenSocket))
        return;

    // Accept incoming connections and put each client on thread
    while (1) {
        if (!acceptingSocket(&acceptSocket, listenSocket, (SOCKADDR *)&clientAddr))
            return;

        sw->updateClients(inet_ntoa(clientAddr.sin_addr));
        clientMap.insert(std::pair<SOCKET, std::string>(acceptSocket, inet_ntoa(clientAddr.sin_addr)));
        //printf("Client %s connected\n", inet_ntoa(clientAddr.sin_addr));

        CreateThread(NULL, 0, tcpClient, &acceptSocket, 0, NULL);
    }

}

DWORD WINAPI tcpClient(void *arg) {
    SOCKET *clientSck = (SOCKET *)arg;

    // Do stuff

    return 0;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void runTCPServer(ServerWindow *sw, int port)
--                     ServerWindow *sw: GUI to update
--                     int port: port to bind to
--
--  RETURNS:       Struct containing all addressing information
--
--  DATE:          March 19, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac
--
--  NOTES:
--      Starts up a UDP server.  Will be responsible for streaming audio.
---------------------------------------------------------------------------------------*/
void runUDPServer(ServerWindow *sw, int port) {
    SOCKADDR_IN addr;
    SOCKET acceptSocket;

    // Init address info
    addr = serverCreateAddress(port);

    // Create a socket for incomming data
    if ((acceptSocket = createSocket(SOCK_DGRAM, IPPROTO_UDP)) == NULL)
        return;

    // bind the socket
    if (!bindSocket(acceptSocket, &addr))
        return;

    while (1) {
        // Do stuff
    }
}
