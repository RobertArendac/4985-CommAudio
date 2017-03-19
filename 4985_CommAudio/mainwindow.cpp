/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	mainwindow.cpp
--
--	FUNCTIONS:
--
--	DATE:			March 3, 2017
--
--	DESIGNERS: Alex Zielinski
--
--	PROGRAMMERS: Alex Zielinski
--
--	NOTES:
--      This is the main window of the program as it is the first window of the program.
--      This window prompts the user if they want to be a client or a server via two
--      buttons labeled respectivly in the window. Clicking on the Client button will
--      redirect the user to the client window. Clicking on the server window will
--      redirect the user to the server  window
---------------------------------------------------------------------------------------*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serverwindow.h"
#include "clientwindow.h"

/* constructor */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this); // display MainWindow
}

/* destructor */
MainWindow::~MainWindow()
{
    delete ui;
}

/* server button event handler */
void MainWindow::on_serverButton_clicked()
{
    this->hide(); // hide the current window
    // create and show server window
    ServerWindow *sw = new ServerWindow();
    sw->show();
}

/* client button event handler */
void MainWindow::on_clientButton_clicked()
{
    this->hide(); // hide the current window
    // create and show the client window
    ClientWindow *cw = new ClientWindow();
    cw->show();
}
