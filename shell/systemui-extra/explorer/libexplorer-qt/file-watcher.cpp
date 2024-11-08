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

#include "file-watcher.h"
#include "gerror-wrapper.h"

#include "file-label-model.h"

#include <QUrl>
#include "file-utils.h"
#include "file-operation-manager.h"
#include "file-info.h"
#include "volume-manager.h"
#include "volumeManager.h"
#include <QDebug>
#include <QStandardPaths>

using namespace Peony;

FileWatcher::FileWatcher(QString uri, QObject *parent, bool isWatchMovesFlag)
    : QObject(parent), m_isWatchMovesFlag(isWatchMovesFlag)
{
    if (uri.startsWith("thumbnail://"))
        return;

    m_uri = uri;
    m_target_uri = uri;
    m_file = g_file_new_for_uri(uri.toUtf8().constData());
    m_cancellable = g_cancellable_new();

    //monitor target file if existed.
    prepare();

    creatorMonitor();

    FileOperationManager::getInstance()->registerFileWatcher(this);

    // fix 97517, could not monitor filesafe box directory changement after unmounted and mount again.
    connect(this, &FileWatcher::directoryUnmounted, this, [=](const QString &uri){
        if (FileUtils::isSamePath(m_uri, uri)) {
            if (m_monitor) {
                //g_file_monitor_cancel(m_monitor);
                g_object_unref(m_monitor);
                m_monitor = nullptr;
            }
            if (m_dir_monitor) {
                //g_file_monitor_cancel(m_dir_monitor);
                g_object_unref(m_dir_monitor);
                m_dir_monitor = nullptr;
            }
        }
    });
}

FileWatcher::~FileWatcher()
{
    FileOperationManager::getInstance()->unregisterFileWatcher(this);

    disconnect();
    //qDebug()<<"~FileWatcher"<<m_uri;
    stopMonitor();
    cancel();

    if (m_cancellable)
        g_object_unref(m_cancellable);
    if (m_dir_monitor)
        g_object_unref(m_dir_monitor);
    if (m_monitor)
        g_object_unref(m_monitor);
    if (m_file)
        g_object_unref(m_file);
}

/*!
 * \brief FileWatcher::prepare
 * <br>
 * If file handle has target uri, we need monitor file that target uri point to.
 * FileWatcher::prepare() just query if there is a target uri of current file handle,
 * and it asumed that everything is no problem. If you want to use
 * a file watcher instance, I recommend you call a file enumerator class instance
 * with FileEnumerator::prepare() and wait it finished first.
 * </br>
 * \see FileEnumerator::prepare().
 */
void FileWatcher::prepare()
{
    auto fileInfo = FileInfo::fromGFile(m_file);
    auto targetUri = fileInfo.get()->targetUri();

    if (!targetUri.isNull()) {
        g_object_unref(m_file);
        m_file = g_file_new_for_uri(targetUri.toUtf8().constData());
        m_target_uri = targetUri;
    }
    return;
}

void FileWatcher::cancel()
{
    if (!m_cancellable)
        return;
    g_cancellable_cancel(m_cancellable);
    g_object_unref(m_cancellable);
    m_cancellable = g_cancellable_new();
}

void FileWatcher::startMonitor()
{
    //make sure only connect once in a watcher.
    stopMonitor();
    if (m_monitor) {
        m_file_handle = g_signal_connect(m_monitor, "changed", G_CALLBACK(file_changed_callback), this);
    }
    if (m_dir_monitor) {
        m_dir_handle = g_signal_connect(m_dir_monitor, "changed", G_CALLBACK(dir_changed_callback), this);
    }

    connect(FileLabelModel::getGlobalModel(), &FileLabelModel::fileLabelChanged, this, [=](const QString &uri) {
        auto parentUri = FileUtils::getParentUri(uri);
        QUrl parentUrl = parentUri;
        QString originalUri = FileUtils::getOriginalUri(m_uri);
        QString homeUri = "file://" +  QStandardPaths::writableLocation(QStandardPaths::HomeLocation);/* 家目录 */
        bool favoriteCond  = (m_uri=="favorite:///" && parentUri == homeUri);/* 快速访问下 */
        bool filesafeCond = (uri.startsWith("filesafe:///") && originalUri.startsWith("file:///"));
        bool labelCond =  uri.startsWith("label://");/* 标识模式下 */
        if (parentUri == m_uri || parentUri == m_target_uri || parentUrl.toDisplayString() == m_uri
            || parentUrl.toDisplayString() == m_target_uri || parentUri == originalUri || favoriteCond
            || filesafeCond||labelCond) {
            Q_EMIT fileChanged(uri);
            qDebug()<<"file label changed"<<uri;
        }
    });

    connect(VolumeManager::getInstance(), &VolumeManager::fileUnmounted, this, [=](const QString &uri){
        auto fixedUri = uri;
        if (m_uri == uri || m_target_uri == uri || m_uri + "/" == fixedUri) {
            Q_EMIT directoryUnmounted(uri);
        }
    });
    connect(Experimental_Peony::VolumeManager::getInstance(), &Experimental_Peony::VolumeManager::signal_unmountFinished, this, [=](const QString &uri){
        /* volume卸载完成跳转到计算机目录(只有进入volume内部目录，卸载时才跳转) */
        auto fixedUri = uri;
        if (m_uri.contains(uri)||m_target_uri.contains(uri)|| m_uri + "/" == fixedUri) {
            Q_EMIT directoryUnmounted(uri);
        }
    });
}

void FileWatcher::stopMonitor()
{
    disconnect(FileLabelModel::getGlobalModel(), &FileLabelModel::fileLabelChanged, this, 0);
    disconnect(VolumeManager::getInstance(), &VolumeManager::fileUnmounted, this, 0);
    disconnect(Experimental_Peony::VolumeManager::getInstance(), &Experimental_Peony::VolumeManager::signal_unmountFinished, this, 0);
    if (m_file_handle > 0) {
        g_signal_handler_disconnect(m_monitor, m_file_handle);
        m_file_handle = 0;
    }
    if (m_dir_handle > 0) {
        g_signal_handler_disconnect(m_dir_monitor, m_dir_handle);
        m_dir_handle = 0;
    }
}

void FileWatcher::forceChangeMonitorDirectory(const QString &uri)
{
    changeMonitorUri(uri);
}

void FileWatcher::creatorMonitor()
{
    GError *err1 = nullptr;
    m_monitor = g_file_monitor_file(m_file,
                                    GFileMonitorFlags(G_FILE_MONITOR_WATCH_MOVES | G_FILE_MONITOR_WATCH_MOUNTS),
                                    m_cancellable,
                                    &err1);
    if (err1) {
        qDebug()<<err1->code<<err1->message;
        g_error_free(err1);
        m_support_monitor = false;
    }

    GError *err2 = nullptr;
    GFileMonitorFlags flag = m_isWatchMovesFlag?G_FILE_MONITOR_WATCH_MOVES:G_FILE_MONITOR_NONE;
    m_dir_monitor = g_file_monitor_directory(m_file,
                    flag,
                    m_cancellable,
                    &err2);
    if (err2) {
        qDebug()<<err2->code<<err2->message;
        g_error_free(err2);
        m_support_monitor = false;
    }
}

void FileWatcher::changeMonitorUri(QString uri)
{
    QString oldUri = m_uri;

    stopMonitor();
    cancel();

    m_uri = uri;
    m_target_uri = uri;
    if (m_file)
        g_object_unref(m_file);
    if (m_monitor)
        g_object_unref(m_monitor);
    if (m_dir_monitor)
        g_object_unref(m_dir_monitor);

    m_file = g_file_new_for_uri(uri.toUtf8().constData());

    prepare();

    creatorMonitor();

    startMonitor();

    Q_EMIT locationChanged(oldUri, m_uri);
}

void FileWatcher::file_changed_callback(GFileMonitor *monitor,
                                        GFile *file,
                                        GFile *other_file,
                                        GFileMonitorEvent event_type,
                                        FileWatcher *p_this)
{
    //qDebug()<<"file_changed_callback"<<event_type;
    //FIXME: when a volume unmounted, the delete signal
    //will be sent, but the volume may not be deleted (in computer:///).
    //I need deal with this case.
    Q_UNUSED(monitor);
    Q_UNUSED(file);
    switch (event_type) {
    case G_FILE_MONITOR_EVENT_MOVED_IN:
    case G_FILE_MONITOR_EVENT_MOVED_OUT:
    case G_FILE_MONITOR_EVENT_RENAMED: {
        char *new_uri = g_file_get_uri(other_file);
        QString uri = new_uri;
        //QUrl url =  uri;
        //uri = url.toDisplayString();
        g_free(new_uri);
        p_this->changeMonitorUri(uri);
        break;
    }
    case G_FILE_MONITOR_EVENT_DELETED: {
        auto uri = g_file_get_uri(file);
        bool shouldSendSignal = p_this->m_target_uri == uri;
        if (uri)
            g_free(uri);
        if (!shouldSendSignal)
            return;
        p_this->stopMonitor();
        p_this->cancel();
        //qDebug()<<p_this->m_target_uri;
        Q_EMIT p_this->directoryDeleted(p_this->m_target_uri);
        break;
    }
    case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED: {
        char *uri = g_file_get_uri(file);
        //qDebug()<<uri;
        Q_EMIT p_this->fileChanged(uri);
        g_free(uri);
        break;
    }
    case G_FILE_MONITOR_EVENT_UNMOUNTED: {
        char *uri = g_file_get_uri(file);
        QString deletedFileUri = uri;
        //QUrl url = deletedFileUri;
        //deletedFileUri = url.toDisplayString();
        g_free(uri);
        Q_EMIT p_this->directoryUnmounted(deletedFileUri);
        break;
    }
    default:
        break;
    }
}

void FileWatcher::dir_changed_callback(GFileMonitor *monitor,
                                       GFile *file,
                                       GFile *other_file,
                                       GFileMonitorEvent event_type,
                                       FileWatcher *p_this)
{
    //qDebug()<<"dir_changed_callback";
    Q_UNUSED(monitor);
    Q_UNUSED(other_file);
    switch (event_type) {
    case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED:
    case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT: {
        if (p_this->m_montor_children_change) {
            char *uri = g_file_get_uri(file);
            QString changedFileUri = uri;
            //qDebug()<<uri;
            //QUrl url = changedFileUri;
            //changedFileUri = url.toDisplayString();
            g_free(uri);
            Q_EMIT p_this->fileChanged(changedFileUri);
        }
        break;
    }
    case G_FILE_MONITOR_EVENT_CREATED:
    case G_FILE_MONITOR_EVENT_MOVED_IN: {
        char *uri = g_file_get_uri(file);
        QString createdFileUri = uri;
        //qDebug()<<"***create uri***"<<createdFileUri;
        g_free(uri);

        Q_EMIT p_this->fileCreated(createdFileUri);
        break;
    }
    case G_FILE_MONITOR_EVENT_DELETED:
    case G_FILE_MONITOR_EVENT_MOVED_OUT: {
        char *uri = g_file_get_uri(file);
        QString deletedFileUri = uri;
        //qDebug()<<"***delete uri***"<<deletedFileUri;
        g_free(uri);

        Q_EMIT p_this->fileDeleted(deletedFileUri);
        break;
    }

    case G_FILE_MONITOR_EVENT_RENAMED: {
        /*!
         * \bug
         * renaming a desktop file can not get new uri correctly.
         *
         * we have to consider trigger it by another way.
         */
        char *old_uri = g_file_get_uri (file);
        char *new_uri = g_file_get_uri(other_file);
        Q_EMIT p_this->fileRenamed(old_uri,new_uri);
        qDebug()<<"***oldUri***newUri***"<<old_uri<<" "<<new_uri;
        g_free(old_uri);
        g_free(new_uri);
        break;
    }
    case G_FILE_MONITOR_EVENT_UNMOUNTED: {
        char *uri = g_file_get_uri(file);
        QString deletedFileUri = uri;
        //QUrl url = deletedFileUri;
        //deletedFileUri = url.toDisplayString();
        g_free(uri);
        //Q_EMIT p_this->directoryUnmounted(deletedFileUri);
        Q_EMIT p_this->fileChanged(deletedFileUri);
        if (FileUtils::urlDecode(deletedFileUri) == FileUtils::urlDecode(p_this->m_uri)) {
            Q_EMIT p_this->directoryUnmounted(deletedFileUri);
        }
        break;
    }
    default:
        break;
    }
}
