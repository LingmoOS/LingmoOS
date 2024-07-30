/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <QDebug>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QUrl>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    if (app.arguments().count() <= 1 || app.arguments().last() == QLatin1String("--help")) {
        qDebug() << "please use ./openurltest <url>";
        return 0;
    }
    QDesktopServices::openUrl(QUrl(app.arguments().last()));

    QMetaObject::invokeMethod(&app, "quit", Qt::QueuedConnection);
    return app.exec();
}
