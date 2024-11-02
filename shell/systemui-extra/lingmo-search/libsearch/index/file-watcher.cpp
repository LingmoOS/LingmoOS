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
#include "file-watcher.h"
using namespace LingmoUISearch;
FileWatcher::FileWatcher(QObject *parent) : QObject(parent), m_config(FileIndexerConfig::getInstance())
{
    qRegisterMetaType<QVector<PendingFile>>("QVector<PendingFile>");
    m_watcher = new FileSystemWatcher(true,
                                      FileSystemWatcher::WatchEvents(FileSystemWatcher::EventMove | FileSystemWatcher::EventMoveSelf |
                                                                     FileSystemWatcher::EventCreate | FileSystemWatcher::EventDelete |
                                                                     FileSystemWatcher::EventDeleteSelf | FileSystemWatcher::EventUnmount |
                                                                     FileSystemWatcher::EventModify | FileSystemWatcher::EventAttributeChange),
                                      FileSystemWatcher::WatchFlags(), nullptr);
    m_pendingFileQueue = PendingFileQueue::getInstance();

    connect(m_watcher, &FileSystemWatcher::created, this, &FileWatcher::onFileCreated);
    connect(m_watcher, &FileSystemWatcher::moveTo, this, &FileWatcher::onFileMoveTo);
    connect(m_watcher, &FileSystemWatcher::modified, this, &FileWatcher::onFileModified);
    connect(m_watcher, &FileSystemWatcher::deleted, this, &FileWatcher::onFileDeletedOrMoved);
    connect(m_watcher, &FileSystemWatcher::moved, this, &FileWatcher::onFileDeletedOrMoved);
    connect(m_watcher, &FileSystemWatcher::unmounted, this, &FileWatcher::onFileDeletedOrMoved);
    connect(m_watcher, &FileSystemWatcher::accessibleChanged, this, &FileWatcher::onFileAccessibleChanged);

    connect(m_pendingFileQueue, &PendingFileQueue::filesUpdate, this, &FileWatcher::filesUpdate);
}

FileWatcher::~FileWatcher()
{
    if(m_watcher) {
        delete m_watcher;
        m_watcher = nullptr;
    }
}

void FileWatcher::addWatch(const QString &path, const QStringList &blackList)
{
    m_watcher->addWatchWithBlackList(QStringList(path), blackList);
}

void FileWatcher::removeWatch(const QString &path, bool updateIndex)
{
    QStringList paths = m_watcher->removeWatch(path);
    if(updateIndex) {
        for(QString &pathToDelete : paths) {
            PendingFile file(pathToDelete);
            file.setIsDir();
            file.setDeleted();
            m_pendingFileQueue->enqueue(file);
        }
    }
}

void FileWatcher::installWatches()
{
    m_watcher->addWatchWithBlackList(m_config->currentIndexableDir(), m_config->currentBlackListOfIndex());
    qDebug() << "Add watch ->" << m_config->currentIndexableDir() << "black list" << m_config->currentBlackListOfIndex();
}

void FileWatcher::removeWatch()
{
    m_watcher->clearAll();
}

void FileWatcher::onFileDeletedOrMoved(const QString &path, bool isDir)
{
    PendingFile file(path);
    file.setIsDir(isDir);
    file.setDeleted();
    m_pendingFileQueue->enqueue(file);
}

void FileWatcher::onFileCreated(const QString &path, bool isDir)
{
    PendingFile file(path);
    file.setIsDir(isDir);
    file.setCreated();
    m_pendingFileQueue->enqueue(file);
}

void FileWatcher::onFileMoveTo(const QString &path, bool isDir)
{
    PendingFile file(path);
    file.setIsDir(isDir);
    file.setMoveTo();
    m_pendingFileQueue->enqueue(file);
}

void FileWatcher::onFileModified(const QString &path)
{
    PendingFile file(path);
    file.setModified();
    m_pendingFileQueue->enqueue(file);
}

void FileWatcher::onFileAccessibleChanged(const QString &path, bool isDir, bool accessible)
{
    PendingFile file(path);
    file.setIsDir(isDir);
    if(accessible) {
       file.setModified();
    } else {
        file.setDeleted();
    }
    m_pendingFileQueue->enqueue(file);
}
