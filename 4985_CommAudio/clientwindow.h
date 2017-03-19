#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>

namespace Ui {
class ClientWindow;
}

class ClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ClientWindow(QWidget *parent = 0);
    ~ClientWindow();

private slots:
    void on_pushButton_clicked();
    void on_cltSelectAllButton_clicked();
    void on_cltUpdateButton_clicked();
    void on_cltUploadButton_clicked();
    void on_cltPlaySelectedTrackButton_clicked();
    void on_cltDownloadSelectedTrackButton_clicked();

private:
    Ui::ClientWindow *ui;
};

#endif // CLIENTWINDOW_H
