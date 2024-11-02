/*
 * Peony-Qt
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 *
 */

#include <QThread>
#include <QTimeLine>
#include "trash-cleaned-watcher.h"
#include "file-watcher.h"

using namespace Peony;

static TrashCleanedWatcher * istrashcleaned_instance = nullptr;
TrashCleanedWatcher::TrashCleanedWatcher()
{
    m_syncThread = new QThread();
    m_watchThread = new QThread();
    m_trash_watcher = std::make_shared<FileWatcher>("trash:///");

    // 将对象移动到线程中
    moveToThread(m_syncThread);
    m_trash_watcher->moveToThread(m_watchThread);

    connect(m_watchThread, &QThread::started, this, &TrashCleanedWatcher::startWatching);

    m_syncThread->start();
    m_watchThread->start();
}

TrashCleanedWatcher::~TrashCleanedWatcher()
{
    if (m_trash_watcher) {
        m_trash_watcher->stopMonitor();
    }

    if(m_watchThread){
        if (m_watchThread->isRunning()) {
            m_watchThread->quit();
            m_watchThread->wait();
        }
        m_watchThread->deleteLater();
        m_watchThread = nullptr;
    }

    if(m_syncThread){
        if (m_syncThread->isRunning()) {
            m_syncThread->quit();
            m_syncThread->wait();
        }
        m_syncThread->deleteLater();
        m_syncThread = nullptr;
    }
}

TrashCleanedWatcher *TrashCleanedWatcher::getInstance()
{
    if(!istrashcleaned_instance){
        istrashcleaned_instance = new TrashCleanedWatcher;
    }
    return istrashcleaned_instance;
}

void TrashCleanedWatcher::startWatching()
{
    // 初始化定时器
    m_batchTimer = new QTimeLine(100, this);
    connect(m_batchTimer, &QTimeLine::finished, this, &TrashCleanedWatcher::checkRecycleBin);

    m_trash_watcher->startMonitor();
    connect(m_trash_watcher.get(), &FileWatcher::fileChanged, this, &TrashCleanedWatcher::startCheck);

}

void TrashCleanedWatcher::startCheck()
{
    if (m_batchTimer->state() != QTimeLine::Running) {
        m_batchTimer->start();
    } else {
        m_batchTimer->setCurrentTime(0);
    }
}

void TrashCleanedWatcher::checkRecycleBin()
{
    GFile *file = g_file_new_for_uri("trash:///");
    GFileInfo *info = g_file_query_info (file,
                                         G_FILE_ATTRIBUTE_TRASH_ITEM_COUNT,
                                         G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                         nullptr,
                                         nullptr);
    auto num = g_file_info_get_attribute_uint32(info,G_FILE_ATTRIBUTE_TRASH_ITEM_COUNT);
    if((num != m_fileNum) && (0 == num || 0 >= m_fileNum)){
        Q_EMIT updateTrashIcon();
    }
    m_fileNum = num;
}
