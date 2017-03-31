/* clientwindow.h */

#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <WinSock2.h>
#include <Windows.h>
#include <QMainWindow>
#include <QMessageBox>
#include <QKeyEvent>
#include <QShortcut>

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
    void updateClientStatus(QString status);

private slots:
    void on_cltConnect_clicked();
    void on_cltSelectAllButton_clicked();
    void on_cltUpdateButton_clicked();
    void on_cltUploadButton_clicked();
    void on_cltPlaySelectedTrackButton_clicked();
    void on_cltDownloadSelectedTrackButton_clicked();
    void keyPressEvent(QKeyEvent* e);
    void keyReleaseEvent(QKeyEvent* e);

private:
    Ui::ClientWindow *ui;
};

#endif // CLIENTWINDOW_H
