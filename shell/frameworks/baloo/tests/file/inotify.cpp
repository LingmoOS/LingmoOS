/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <iostream>

#include "kinotify.h"
#include "util.h"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    KInotify inotify(nullptr /*no config*/);
    QObject::connect(&inotify, &KInotify::installedWatches,
                     &app, &QCoreApplication::quit);

    QObject::connect(&inotify, &KInotify::attributeChanged,
                     [](const QString& fileUrl) { qDebug() << "AttrbuteChanged:" << fileUrl; });
    QObject::connect(&inotify, &KInotify::created,
                     [](const QString& fileUrl, bool isDir) { qDebug() << "Created:" << fileUrl << isDir; });
    QObject::connect(&inotify, &KInotify::deleted,
                     [](const QString& fileUrl, bool isDir) { qDebug() << "Deleted:" << fileUrl << isDir; });
    QObject::connect(&inotify, &KInotify::modified,
                     [](const QString& fileUrl) { qDebug() << "Modified:" << fileUrl; });
    QObject::connect(&inotify, &KInotify::closedWrite,
                     [](const QString& fileUrl) { qDebug() << "ClosedWrite:" << fileUrl; });

    QElapsedTimer timer;
    timer.start();

    KInotify::WatchEvents flags(KInotify::EventMove | KInotify::EventDelete | KInotify::EventDeleteSelf
                                | KInotify::EventCloseWrite | KInotify::EventCreate
                                | KInotify::EventAttributeChange | KInotify::EventModify);
    inotify.addWatch(QDir::homePath(), flags);
    app.exec();

    std::cout << "Elapsed: " << timer.elapsed() << std::endl;
    printIOUsage();
}
