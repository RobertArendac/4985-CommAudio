/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	mainwindow.cpp
--
--	FUNCTIONS:
--
--	DATE:			March 3, 2017
--
--	DESIGNERS:
--
--	PROGRAMMERS:
--
--	NOTES:
--     The main window that will be displayed to the user
---------------------------------------------------------------------------------------*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
