// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "filesystem/dfilewatchermanager.h"
#include <QCoreApplication>
#include <QTemporaryFile>
#include <QDebug>
DCORE_USE_NAMESPACE

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    DFileWatcherManager manager;
    QTemporaryFile tmpfile1;
    tmpfile1.open();
    QFile file1(tmpfile1.fileName());
    QTemporaryFile tmpfile2;
    tmpfile2.open();
    QFile file2(tmpfile2.fileName());

    manager.add(tmpfile1.fileName());
    manager.add(tmpfile2.fileName());

    QObject::connect(&manager, &Dtk::Core::DFileWatcherManager::fileModified, &app, [=](const QString value) {
        qDebug() << "文件发生变动:" << value;
    });

    QObject::connect(&manager, &Dtk::Core::DFileWatcherManager::fileDeleted, &app, [=](const QString value) {
        qDebug() << "文件被删除:" << value;
    });

    file1.open(QIODevice::WriteOnly | QIODevice::Text);
    file1.write("test");
    file1.flush();
    file1.close();

    file2.open(QIODevice::WriteOnly | QIODevice::Text);
    file2.write("test");
    file2.close();

    qDebug() << manager.watchedFiles();
    qDebug() << "---------------------------";
    app.processEvents();
    manager.removeAll();
    qDebug() << manager.watchedFiles();
    return app.exec();
}
