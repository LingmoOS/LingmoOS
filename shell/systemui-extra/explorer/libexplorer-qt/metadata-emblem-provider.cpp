/*
 * Peony-Qt
 *
 * Copyright (C) 2023, KylinSoft Information Technology Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */


#include "metadata-emblem-provider.h"
#include "file-meta-info.h"

#include <QApplication>
#include <QThread>

using namespace Peony;

static MetadataEmblemProvider *global_instance = nullptr;

MetadataEmblemProvider *MetadataEmblemProvider::getInstance()
{
    if (!global_instance)
        global_instance = new MetadataEmblemProvider;
    return global_instance;
}

const QString MetadataEmblemProvider::emblemKey()
{
    return "metainfo-emblem";
}

QStringList MetadataEmblemProvider::getFileEmblemIcons(const QString &uri)
{
    static QThread *uiThread = qApp->thread();
    std::shared_ptr<Peony::FileMetaInfo> metaInfo = nullptr;
    if (QThread::currentThread() == uiThread) {
        metaInfo = FileMetaInfo::fromUri(uri);
    } else {
        // 使用线程+blocking queue connection获取metainfo，原来的方案存在死锁可能性
        auto dupJob = new MetadataDupJob(uri, nullptr);
        dupJob->start();
        dupJob->wait();
        metaInfo = dupJob->dupMetaInfo();
        delete dupJob;
    }
    if(!metaInfo || !metaInfo.get())
        return QStringList();
    return metaInfo->getMetaInfoStringListV1("emblems");
}

MetadataEmblemProvider::MetadataEmblemProvider(QObject *parent) : EmblemProvider(parent)
{
//    connect(this, &MetadataEmblemProvider::requestDupMetaInfo, this, &MetadataEmblemProvider::getDupMetaInfo, Qt::BlockingQueuedConnection);

//    connect(qApp, &QCoreApplication::aboutToQuit, this, [=]{
//        // note:
//        // 目前发现使用blockqueueconnection和emblemjob交互会影响app退出，这里尝试断开连接并且处理所有待处理事件解决此问题
//        // 这个改动对解决#181067 【音乐】音乐进程卡死 有一定帮助
//        disconnect(this, &MetadataEmblemProvider::requestDupMetaInfo, 0, 0);
//        qApp->processEvents();
//    });
}

std::shared_ptr<FileMetaInfo> MetadataEmblemProvider::getDupMetaInfo(const QString &uri)
{
    return FileMetaInfo::dupFromUri(uri);
}

MetadataDupJob::MetadataDupJob(const QString &uri, QObject *parent) : QThread(parent)
{
    m_uri = uri;

    connect(this, &MetadataDupJob::requestDupMetaInfo, qApp, [=](const QString &uri){
        return FileMetaInfo::dupFromUri(uri);
    }, Qt::BlockingQueuedConnection);
}

void MetadataDupJob::run()
{
    m_dupMetaInfo = requestDupMetaInfo(m_uri);
}

std::shared_ptr<FileMetaInfo> MetadataDupJob::dupMetaInfo() const
{
    return m_dupMetaInfo;
}
