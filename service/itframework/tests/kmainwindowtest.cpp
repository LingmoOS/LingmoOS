/*
    SPDX-FileCopyrightText: 2002 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2005-2006 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kmainwindowtest.h"

#include <QApplication>
#include <QLabel>
#include <QMenuBar>
#include <QStatusBar>
#include <QTimer>

MainWindow::MainWindow()
{
    QTimer::singleShot(2 * 1000, this, &MainWindow::showMessage);

    setCentralWidget(new QLabel(QStringLiteral("foo"), this));

    menuBar()->addAction(QStringLiteral("hi"));
}

void MainWindow::showMessage()
{
    statusBar()->show();
    statusBar()->showMessage(QStringLiteral("test"));
}

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("kmainwindowtest"));
    QApplication app(argc, argv);

    MainWindow *mw = new MainWindow; // deletes itself when closed
    mw->show();

    return app.exec();
}
