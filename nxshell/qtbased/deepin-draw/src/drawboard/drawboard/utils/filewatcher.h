// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CMANAGEVIEWSIGLETON_H
#define CMANAGEVIEWSIGLETON_H

#include "globaldefine.h"

#include <QList>
#include <QFileSystemWatcher>
#include <QAtomicInt>
#include <QDebug>
#include <QThread>
#include <QMutex>

//#ifdef USE_DTK
//DWIDGET_USE_NAMESPACE
//#endif

class DRAWLIB_EXPORT SystemFileWatcher:
#ifdef Q_OS_LINUX
    public QThread
#else
    public QFileSystemWatcher
#endif
{
    Q_OBJECT
public:
    enum EFileChangedType {EFileModified, EFileMoved, EFileCount};

    explicit SystemFileWatcher(QObject *parent = nullptr);
    ~SystemFileWatcher();

    bool isValid();

    void addWather(const QString &path);
    void removePath(const QString &path);

    void clear();

signals:
    void fileChanged(const QString &path, int tp);

#ifdef Q_OS_LINUX
protected:
    void run();

private:
    void doRun();


    int  _handleId = -1;
    bool _running = false;



    QMutex _mutex;
    QMap<int, QString> watchedFilesId;
    QMap<QString, int> watchedFiles;
#else
private:
    QSet<QString> watchedFiles;
#endif


};

#endif // CDRAWPARAMSIGLETON_H
