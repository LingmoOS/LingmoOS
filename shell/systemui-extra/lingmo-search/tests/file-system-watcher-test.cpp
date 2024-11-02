/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#include "file-system-watcher-test.h"
#include <QDebug>
using namespace LingmoUISearch;
FileSystemWatcherTest::FileSystemWatcherTest(QObject *parent) : QObject(parent)
{
    m_watcher = new FileSystemWatcher(true, FileSystemWatcher::WatchEvents(FileSystemWatcher::EventMove | FileSystemWatcher::EventMoveSelf |
                                                              FileSystemWatcher::EventCreate | FileSystemWatcher::EventDelete |
                                                              FileSystemWatcher::EventDeleteSelf | FileSystemWatcher::EventUnmount |
                                                              FileSystemWatcher::EventModify | FileSystemWatcher::EventAttributeChange));
}

void FileSystemWatcherTest::beginSignalTest()
{
    m_watcher->addWatch("");

    connect(m_watcher, &FileSystemWatcher::attributeChanged,
                     [](const QString& fileUrl) { qDebug() << "AttrbuteChanged:" << fileUrl; });

    connect(m_watcher, &FileSystemWatcher::created,
                     [](const QString& fileUrl, bool isDir) { qDebug() << "Created:" << fileUrl << isDir; });

    connect(m_watcher, &FileSystemWatcher::deleted,
                     [](const QString& fileUrl, bool isDir) { qDebug() << "Deleted:" << fileUrl << isDir; });

    connect(m_watcher, &FileSystemWatcher::modified,
                     [](const QString& fileUrl) { qDebug() << "Modified:" << fileUrl; });

    connect(m_watcher, &FileSystemWatcher::moved,
                     [](const QString& fileUrl, bool isDir) { qDebug() << "moved:" << fileUrl << isDir; });

    connect(m_watcher, &FileSystemWatcher::closedWrite,
                     [](const QString& fileUrl) { qDebug() << "ClosedWrite:"  << fileUrl; });
    connect(m_watcher, &FileSystemWatcher::moveTo,
                     [](const QString& fileUrl) { qDebug() << "moveTo:"  << fileUrl; });
}
