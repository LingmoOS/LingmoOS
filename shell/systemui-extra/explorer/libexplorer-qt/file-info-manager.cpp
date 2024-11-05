/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "file-info-manager.h"
#include "thumbnail-manager.h"
#include <gio/gio.h>
#include <QMap>
#include <QDebug>
#include <QMutexLocker>

using namespace Peony;

static FileInfoManager* global_file_info_manager = nullptr;
static QHash<QString, std::weak_ptr<FileInfo>> *global_info_list = nullptr;
static QHash<QString, std::weak_ptr<ExtraInfoRecorder>> *global_extraInfo_list = nullptr;/* <uri, ExtraInfoRecorder>,暂时只允许在fileitem中使用，慎用！！！ */

static bool g_is_auto_parted = false;

static QMutex m_op_lock;

FileInfoManager::FileInfoManager()
{
    global_info_list = new QHash<QString, std::weak_ptr<FileInfo>>();
    global_extraInfo_list = new QHash<QString, std::weak_ptr<ExtraInfoRecorder>>();

    GFile *file = g_file_new_for_uri("file:///data/usershare");
    g_is_auto_parted = g_file_query_exists(file, nullptr);
    g_object_unref(file);
}

FileInfoManager::~FileInfoManager()
{
    delete global_info_list;
    delete global_extraInfo_list;
}

FileInfoManager *FileInfoManager::getInstance()
{
    if (!global_file_info_manager)
        global_file_info_manager = new FileInfoManager;
    return global_file_info_manager;
}

std::shared_ptr<FileInfo> FileInfoManager::findFileInfoByUri(const QString &uri)
{
    Q_ASSERT(global_info_list);
    m_op_lock.lock();
    auto info = global_info_list->value(uri).lock();//.lock();
    m_op_lock.unlock();
    return info;
}

bool FileInfoManager::isAutoParted()
{
    return g_is_auto_parted;
}

std::shared_ptr<FileInfo> FileInfoManager::insertFileInfo(std::shared_ptr<FileInfo> info)
{
    Q_ASSERT(global_info_list);

    m_op_lock.lock();

    if (global_info_list->value(info->uri()).lock()) {
        //qDebug()<<"has info yet"<<info->uri();
        info = global_info_list->value(info->uri()).lock();
    } else {
        global_info_list->insert(info->uri(), info);
    }

    m_op_lock.unlock();

    return info;
}

void FileInfoManager::updateFileInfo(std::shared_ptr<FileInfo>& info)
{
    Q_ASSERT(global_info_list);
    QMutexLocker mtx(&m_op_lock);
    QString uri = info->uri();
    if(global_info_list->value(uri).lock()){
        std::shared_ptr<FileInfo> fileInfo = global_info_list->value(uri).lock();
        fileInfo.get()->FileInfo::operator=(*info.get());/* update file info */
        info = fileInfo;
    }
    global_info_list->insert(uri, info);
    return;

}

std::shared_ptr<ExtraInfoRecorder> FileInfoManager::getExtraInfoRecorderByUri(const QString &uri)
{
    Q_ASSERT(global_extraInfo_list);
    QMutexLocker mtx(&m_op_lock);
    auto tmp = global_extraInfo_list->value(uri).lock();
    if (!tmp) {
        tmp = std::make_shared<ExtraInfoRecorder>(uri);
        global_extraInfo_list->insert(uri, tmp);
    }
    return tmp;
}

void FileInfoManager::showState()
{
    qDebug()<<global_info_list->keys().count()<<global_info_list->values().count();
}
