/*
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntulingmo.com/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FILESYSTEMWATCHER_H
#define FILESYSTEMWATCHER_H

#include <QObject>
#include <QSet>
#include <QSocketNotifier>

class FileSystemWatcher : public QObject
{
    Q_OBJECT
public:
    static FileSystemWatcher *instance();

    FileSystemWatcher(QObject *parent = 0);
    ~FileSystemWatcher();

    bool watcherInitSuccess();

signals:
    void deviceAdded(const QString &addDevice);
    void deviceRemoved(const QString &removeDevice);

public slots:
    bool initWatcher();
    bool clearWatcher();

private slots:
    void onMountDeviceFileContentsChanged();

private:
    int m_fd;
    QSocketNotifier *m_socketNotifier = nullptr;
    QSet<QString> m_origFileSet;
    static FileSystemWatcher *m_watcher;
};

#endif // FILESYSTEMWATCHER_H
