#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QMainWindow>

namespace Ui {
class ServerWindow;
}

class ServerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ServerWindow(QWidget *parent = 0);
    ~ServerWindow();

private slots:
    void on_srvStartStopButton_clicked();

    void on_srvTrackPreviousButton_clicked();

    void on_srvTrackRWButton_clicked();

    void on_srvTrackPlayPauseButton_clicked();

    void on_srvTrackFFButton_clicked();

    void on_srvTrackNextButton_clicked();

    void on_srvShuffleRadioButton_clicked();

    void on_srvPlaySelectedTrackButton_clicked();

private:
    Ui::ServerWindow *ui;
};

#endif // SERVERWINDOW_H
