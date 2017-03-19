#include "serverwindow.h"
#include "ui_serverwindow.h"

ServerWindow::ServerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ServerWindow)
{
    ui->setupUi(this);
}

ServerWindow::~ServerWindow()
{
    delete ui;
}

void ServerWindow::on_srvStartStopButton_clicked()
{

}

void ServerWindow::on_srvTrackPreviousButton_clicked()
{

}

void ServerWindow::on_srvTrackRWButton_clicked()
{

}

void ServerWindow::on_srvTrackPlayPauseButton_clicked()
{

}

void ServerWindow::on_srvTrackFFButton_clicked()
{

}

void ServerWindow::on_srvTrackNextButton_clicked()
{

}

void ServerWindow::on_srvShuffleRadioButton_clicked()
{

}

void ServerWindow::on_srvPlaySelectedTrackButton_clicked()
{

}
