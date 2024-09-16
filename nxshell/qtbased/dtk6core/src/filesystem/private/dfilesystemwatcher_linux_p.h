// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILESYSTEMWATCHER_P_H
#define DFILESYSTEMWATCHER_P_H

#include "dobject_p.h"

#include <QSocketNotifier>
#include <QHash>
#include <QMap>

DCORE_BEGIN_NAMESPACE

class DFileSystemWatcher;
class DFileSystemWatcherPrivate : public DObjectPrivate
{
    Q_DECLARE_PUBLIC(DFileSystemWatcher)

public:
    DFileSystemWatcherPrivate(int fd, DFileSystemWatcher *qq);
    ~DFileSystemWatcherPrivate();

    QStringList addPaths(const QStringList &paths, QStringList *files, QStringList *directories);
    QStringList removePaths(const QStringList &paths, QStringList *files, QStringList *directories);

    QStringList files, directories;
    int inotifyFd;
    QHash<QString, int> pathToID;
    QMultiHash<int, QString> idToPath;
    QSocketNotifier notifier;

    // private slots
    void _q_readFromInotify();

private:
    void onFileChanged(const QString &path, bool removed);
    void onDirectoryChanged(const QString &path, bool removed);
};

DCORE_END_NAMESPACE

#endif // DFILESYSTEMWATCHER_P_H
