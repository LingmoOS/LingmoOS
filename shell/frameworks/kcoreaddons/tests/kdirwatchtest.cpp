/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 1998 Sven Radej <sven@lisa.exp.univie.ac.at>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kdirwatchtest.h"

#include <QCoreApplication>
#include <QStringList>

#include <QDebug>

// TODO debug crash when calling "./kdirwatchtest ./kdirwatchtest"

int main(int argc, char **argv)
{
    // TODO port to QCommandLineArguments once it exists
    // options.add("+[directory ...]", qi18n("Directory(ies) to watch"));

    QCoreApplication a(argc, argv);

    myTest testObject;

    KDirWatch *dirwatch1 = KDirWatch::self();
    KDirWatch *dirwatch2 = new KDirWatch;

    testObject.connect(dirwatch1, &KDirWatch::dirty, &myTest::dirty);
    testObject.connect(dirwatch1, &KDirWatch::created, &myTest::created);
    testObject.connect(dirwatch1, &KDirWatch::deleted, &myTest::deleted);

    // TODO port to QCommandLineArguments once it exists
    const QStringList args = a.arguments();
    for (int i = 1; i < args.count(); ++i) {
        const QString arg = args.at(i);
        if (!arg.startsWith("-")) {
            qDebug() << "Watching: " << arg;
            dirwatch2->addDir(arg);
        }
    }

    QString home = QString(getenv("HOME")) + '/';
    QString desk = home + "Desktop/";
    qDebug() << "Watching: " << home;
    dirwatch1->addDir(home);
    qDebug() << "Watching file: " << home << "foo ";
    dirwatch1->addFile(home + "foo");
    qDebug() << "Watching: " << desk;
    dirwatch1->addDir(desk);
    QString test = home + "test/";
    qDebug() << "Watching: (but skipped) " << test;
    dirwatch1->addDir(test);

    dirwatch1->startScan();
    dirwatch2->startScan();

    if (!dirwatch1->stopDirScan(home)) {
        qDebug() << "stopDirscan: " << home << " error!";
    }
    if (!dirwatch1->restartDirScan(home)) {
        qDebug() << "restartDirScan: " << home << "error!";
    }
    if (!dirwatch1->stopDirScan(test)) {
        qDebug() << "stopDirScan: error";
    }

    KDirWatch::statistics();

    delete dirwatch2;

    KDirWatch::statistics();

    return a.exec();
}

#include "moc_kdirwatchtest.cpp"
