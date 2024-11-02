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

#include "filesystemwatcher.h"
#include "../util.h"

#include <QDebug>

FileSystemWatcher *FileSystemWatcher::m_watcher = NULL;

FileSystemWatcher *FileSystemWatcher::instance()
{
    if (!m_watcher) {
        m_watcher = new FileSystemWatcher;
        m_watcher->initWatcher();
    }
    return m_watcher;
}

FileSystemWatcher::FileSystemWatcher(QObject *parent)
    : QObject(parent)
    ,m_fd(-1)
{

}

FileSystemWatcher::~FileSystemWatcher()
{
    this->clearWatcher();
}

bool FileSystemWatcher::initWatcher()
{
    m_origFileSet = getFileContentsLineByLine(DEVICE_MOUNT_PONINT_RECORD_FILE);
    m_fd = open(DEVICE_MOUNT_PONINT_RECORD_FILE, O_RDONLY);
    if (m_fd == -1) {
        //qDebug() << QString("open %1 failed!").arg(DEVICE_MOUNT_PONINT_RECORD_FILE);
        return false;
    }

    m_socketNotifier = new QSocketNotifier(m_fd, QSocketNotifier::Write, this);
    m_socketNotifier->setEnabled(true);
    connect(m_socketNotifier, &QSocketNotifier::activated, this, &FileSystemWatcher::onMountDeviceFileContentsChanged);
    return true;
}

bool FileSystemWatcher::watcherInitSuccess()
{
    if (m_fd != -1 && m_socketNotifier) {
        return true;
    }
    else {
        return false;
    }
}

bool FileSystemWatcher::clearWatcher()
{
    if (this->watcherInitSuccess()) {
        if (m_fd != -1) {
            close(m_fd);
            m_fd = -1;
        }
        return true;
    }
    else {
        return false;
    }
}

void FileSystemWatcher::onMountDeviceFileContentsChanged()  //emit the signal if the devie is added or removed
{
    QSet<QString> nowFileSet = getFileContentsLineByLine(DEVICE_MOUNT_PONINT_RECORD_FILE);
    for(const QString &mountPath: nowFileSet - m_origFileSet) {
        emit this->deviceAdded(getDeviceMountedPointPath(mountPath));
    }
    for(const QString &mountPath: m_origFileSet - nowFileSet) {
        emit this->deviceRemoved(getDeviceMountedPointPath(mountPath));
    }
    m_origFileSet = nowFileSet;
}
