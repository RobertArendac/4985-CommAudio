/* serverwindow.h */

#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QMainWindow>
#include <WinSock2.h>
#include <Windows.h>
#include <QDebug>

namespace Ui {
class ServerWindow;
}

class ServerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ServerWindow(QWidget *parent = 0);
    ~ServerWindow();

    static DWORD WINAPI udpServerThread(void *arg);
    static DWORD WINAPI tcpServerThread(void *arg);
    void createSongList();
    void updateClients(const char *client);
    void ServerWindow::removeClient(const char *client);
    static QStringList getSongs();
    void updateServerStatus(QString status);
    void playNextTrack();
    void playPrevTrack();

private slots:
    void on_srvStartStopButton_clicked();
    void on_srvTrackPreviousButton_clicked();
    void on_srvTrackRWButton_clicked();
    void on_srvTrackPlayButton_clicked();
    void on_srvTrackFFButton_clicked();
    void on_srvTrackNextButton_clicked();
    void on_srvShuffleRadioButton_clicked();
    void on_srvSelectPlaylistButton_clicked();
    void on_srvTrackStopButton_clicked();

    void on_srvTrackPauseButton_clicked();

private:
    Ui::ServerWindow *ui;
};

#endif // SERVERWINDOW_H
