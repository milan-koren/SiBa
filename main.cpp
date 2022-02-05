#include "mainwindow.h"
#include <QApplication>

/*!
 * *****************************************************************
 *                               SiBa
 * *****************************************************************
 * \file main.cpp
 *
 * \author M. Koren, milan.koren3@gmail.com
 * Source: https:\\github.com/milan-koren/SiBa
 * Licence: EUPL v. 1.2
 * https://joinup.ec.europa.eu/collection/eupl
 * *****************************************************************
 */

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}
