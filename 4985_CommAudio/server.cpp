#include "server.h"
#include "client.h"
#include <map>
#include <WS2tcpip.h>

QAudioFormat format;
QAudioOutput *output;
QBuffer *audioBuffer;
QString audioFilePath;

std::map<SOCKET, std::string> clientMap;

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
        qWarning()<<"raw audio format not supported by backend, cannot play audio.";
        return;
    }

    // initialize output
    output = new QAudioOutput(format);
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
--      Stops audio from playing
---------------------------------------------------------------------------------------*/
void stopAudio()
{
    // check if audio is playing
    if (output->state() == QAudio::ActiveState)
    {
        output->stop(); // stop the audio
        output->reset();
        audioBuffer->close();
    }
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void pauseAudio()
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
--      Suspend audio
---------------------------------------------------------------------------------------*/
void pauseAudio()
{
    // check if audio is playing
    if (output->state() == QAudio::ActiveState)
    {
        output->suspend(); // pause the audio
    }
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void playAudio(QString &filePath)
--                      QString &filePath: the filepath of audio file
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
void playAudio(QString &filePath)
{
    // check if audio was paused
    if(output->state() == QAudio::SuspendedState)
    {
        output->resume();
        //qDebug() << audioBuffer->size();
    }
    else // audio not paused
    {
        // create file handle for audio file
        QFile audioFile(filePath);

        // open audio file
        if (audioFile.open(QIODevice::ReadOnly))
        {   // check if user selected a different track
            if (audioFilePath != filePath)
            {
                audioFilePath = filePath;

                // seek to raw audio data of wav file
                audioFile.seek(AUDIODATA);

                // extract raw audio data
                QByteArray audio = audioFile.readAll();

                // initialize audio buffer
                audioBuffer = new QBuffer(&audio);
                audioBuffer->open(QIODevice::ReadWrite);
                audioBuffer->seek(0);
                //qDebug() << audioBuffer->size();

                output->start(audioBuffer); // play track

                // event loop for tracck
                QEventLoop loop;
                QObject::connect(output, SIGNAL(stateChanged(QAudio::State)), &loop, SLOT(quit()));
                do
                {
                    loop.exec();
                } while(output->state() == QAudio::ActiveState);
            }
        }
    }
}

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
--  PROGRAMMER:    Robert Arendac
--
--  NOTES:
--      Fills an address struct.  IP is any, port is passed in, used for TCP.
---------------------------------------------------------------------------------------*/
SOCKADDR_IN serverCreateAddress(int port)
{
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
--  RETURNS:       void
--
--  DATE:          March 19, 2017
--
--  MODIFIED:      March 29, 2017 - Made function return an int ~ AZ
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    Robert Arendac, Alex Zielinski
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
    {
        sw->updateServerStatus("Status: Socket Error");
        return;
    }

    // Initialize address info
    addr = serverCreateAddress(port);

    // Bind the listening socket
    if (!bindSocket(listenSocket, &addr))
    {
        sw->updateServerStatus("Status: Socket Error");
        return;
    }

    // Set socket to listen for connection
    if (!listenConnection(listenSocket))
    {
        sw->updateServerStatus("Status: Socket Error");
        return;
    }

    sw->updateServerStatus("Status: ON");
    // Accept incoming connections and put each client on thread
    while (1)
    {
        if (!acceptingSocket(&acceptSocket, listenSocket, (SOCKADDR *)&clientAddr))
        {
            return;
        }

        sw->updateClients(inet_ntoa(clientAddr.sin_addr));
        clientMap.insert(std::pair<SOCKET, std::string>(acceptSocket, inet_ntoa(clientAddr.sin_addr)));
        printf("Client %s connected\n", inet_ntoa(clientAddr.sin_addr));

        CreateThread(NULL, 0, tcpClient, &acceptSocket, 0, NULL);
    }
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     DWORD WINAPI tcpClient(void *arg)
--                     void *arg: Socket bound to specific client
--
--  RETURNS:       Thread exit information
--
--  DATE:          March 25, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    Robert Arendac
--
--  NOTES:
--      Thread for handling each client.  Will build a song list and send it out to the
--      client.
---------------------------------------------------------------------------------------*/
DWORD WINAPI tcpClient(void *arg)
{
    SOCKET *clientSck = (SOCKET *)arg;  //Client socket
    std::string songlist;               //String of all songs
    DWORD sendBytes;                    //Bytes to be sent
    SocketInformation *si;              //Struct holding socket info
    char music[SONG_SIZE];              //C-string of songs
    WSAEVENT events[1];                 //Array of events (just one)
    DWORD result;                       //Result of waiting for events

    // Build the song list
    QStringList songs = ServerWindow::getSongs();
    for (auto song : songs)
    {
        songlist += song.toStdString() + "\n";
    }

    // Copy the song list to a c-string, can't send std::string
    strcpy(music, songlist.c_str());

    // Fill out the socket info
    si = (SocketInformation *)malloc(sizeof(SocketInformation));
    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));
    strcpy(si->buffer, music);
    si->socket = *clientSck;
    si->bytesReceived = 0;
    si->bytesSent = 0;
    si->dataBuf.buf = si->buffer;
    si->dataBuf.len = SONG_SIZE;

    // Send the song list
    WSASend(si->socket, &(si->dataBuf), 1, &sendBytes, 0, &(si->overlapped), clientRoutine);

    // Wait for the send to complete
    events[0] = WSACreateEvent();
    if ((result = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE)) != WAIT_IO_COMPLETION)
        fprintf(stdout, "WaitForMultipleEvents() failed: %d", result);

    return 0;
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void CALLBACK clientRoutine(DWORD error, DWORD, LPWSAOVERLAPPED, DWORD)
--                     DWORD error: error that occured during WSASend()
--                     Other args unused
--
--  RETURNS:       void
--
--  DATE:          March 29, 2017
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    Robert Arendac
--
--  NOTES:
--      Completion routine for sending song list.  In this case, we just want to check
--      for error.
---------------------------------------------------------------------------------------*/
void CALLBACK clientRoutine(DWORD error, DWORD, LPWSAOVERLAPPED, DWORD)
{
    if (error)
    {
        fprintf(stderr, "Error: %d\n", error);
    }
}

/*--------------------------------------------------------------------------------------
--  INTERFACE:     void runTCPServer(ServerWindow *sw, int port)
--                     ServerWindow *sw: GUI to update
--                     int port: port to bind to
--
--  RETURNS:       void
--
--  DATE:          March 19, 2017
--
--  MODIFIED:      March 28, 2017 - Added multicasting capabilities
--                 March 29, 2017 - Made function return an int ~ AZ
--
--  DESIGNER:      Robert Arendac
--
--  PROGRAMMER:    RobertArendac, Alex Zielinski
--
--  NOTES:
--      Starts up a UDP server.  Will be responsible for streaming audio.  Also sets up
--      multicasting
---------------------------------------------------------------------------------------*/
void runUDPServer(ServerWindow *sw, int port)
{
    SOCKADDR_IN addr, cltDest;  //Addresses to receive from and send to
    SOCKET acceptSocket;        //Connect to send/receive on
    struct ip_mreq stMreq;      //Struct for multicasting
    u_long ttl = MCAST_TTL;     //Time to live
    int flag = 0;               //False flag

    // Init address info
    addr = serverCreateAddress(port);

    // Create a socket for incomming data
    if ((acceptSocket = createSocket(SOCK_DGRAM, IPPROTO_UDP)) == NULL)
    {
        sw->updateServerStatus("Status: Socket Error");
        return;
    }

    // bind the socket
    if (!bindSocket(acceptSocket, &addr))
    {
        sw->updateServerStatus("Status: Socket Error");
        return;
    }

    // Multicast interface
    stMreq.imr_multiaddr.s_addr = inet_addr(MCAST_ADDR);
    stMreq.imr_interface.s_addr = INADDR_ANY;

    // Join multicast group, specify time-to-live, and disable loop
    if (!setServOptions(acceptSocket, IP_ADD_MEMBERSHIP, (char *)&stMreq))
    {
        sw->updateServerStatus("Status: Socket Error");
        return;
    }

    if (!setServOptions(acceptSocket, IP_MULTICAST_TTL, (char *)&ttl))
    {
        sw->updateServerStatus("Status: Socket Error");
        return;
    }

    if (!setServOptions(acceptSocket, IP_MULTICAST_LOOP, (char *)&flag))
    {
        sw->updateServerStatus("Status: Socket Error");
        return;
    }

    cltDest = clientCreateAddress(MCAST_ADDR, MCAST_PORT);

    sw->updateServerStatus("Status: ON");

    while (1)
    {
        // Do stuff
    }
}
