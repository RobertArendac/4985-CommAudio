/* clientwindow.h */

#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <WinSock2.h>
#include <Windows.h>

namespace Ui {
class ClientWindow;
}

class ClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ClientWindow(QWidget *parent = 0);
    ~ClientWindow();

    static DWORD WINAPI udpClientThread(void *);
    static DWORD WINAPI tcpClientThread(void *);
    void updateSongs(QStringList songs);

private slots:
    void on_cltConnect_clicked();
    void on_cltSelectAllButton_clicked();
    void on_cltUpdateButton_clicked();
    void on_cltUploadButton_clicked();
    void on_cltPlaySelectedTrackButton_clicked();
    void on_cltDownloadSelectedTrackButton_clicked();

private:
    Ui::ClientWindow *ui;
};

#endif // CLIENTWINDOW_H
