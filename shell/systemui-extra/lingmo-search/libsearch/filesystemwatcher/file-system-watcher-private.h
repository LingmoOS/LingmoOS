/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#ifndef FILESYSTEMWATCHERPRIVATE_H
#define FILESYSTEMWATCHERPRIVATE_H

#endif // FILESYSTEMWATCHERPRIVATE_H
#include "file-system-watcher.h"

#include <QHash>
#include <QByteArray>
#include <QSocketNotifier>
#include <QThreadPool>
#include <QMutex>

namespace LingmoUISearch {
class FileSystemWatcherPrivate
{
    friend class FileSystemWatcher;
public:
    FileSystemWatcherPrivate(FileSystemWatcher *parent);
    ~FileSystemWatcherPrivate();

    void addWatchWithBlackList(const QStringList &pathList, const QStringList &blackList);
    QStringList removeWatch(const QString &path);
    QString removeWatch(int wd);
    void clearAll();

private:
    void init();
    void addWatch(const QString &path);

    FileSystemWatcher::WatchEvents m_watchEvents;
    FileSystemWatcher::WatchFlags m_watchFlags;

    int m_inotifyFd = -1;
    QSocketNotifier* m_notifier = nullptr;
    // wd -> url
    QHash<int, QString> m_watchPathHash;
    QStringList m_watchedRootPaths;
    FileSystemWatcher *q = nullptr;
    bool m_recursive = true;

};



}
