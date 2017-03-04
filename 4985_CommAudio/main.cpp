/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	main.cpp
--
--	FUNCTIONS:      int main(int argc, char *argv[])
--
--	DATE:			March 3, 2017
--
--	DESIGNERS:		Qt
--
--	PROGRAMMERS:	Qt
--
--	NOTES:
--     The default Qt main driver.
---------------------------------------------------------------------------------------*/

#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
