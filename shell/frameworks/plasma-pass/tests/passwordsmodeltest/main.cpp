// SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QApplication>

#include "mainwindow.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    MainWindow window;
    window.showNormal();
    return QApplication::exec();
}
