/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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

#include "computer-remote-volume-item.h"
#include "computer-model.h"
#include "file-utils.h"
#include <QMessageBox>

#include <gio/gio.h>

#include <QDebug>

QString queryTargetUri(const QString &uri)
{
    if (uri.startsWith("computer:///")) {
        g_autoptr (GFile) gfile = g_file_new_for_uri(uri.toUtf8().constData());
        g_autoptr (GFileInfo) gfileinfo = g_file_query_info(gfile, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, 0, 0);
        g_autofree gchar* target_uri = g_file_info_get_attribute_as_string(gfileinfo, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
        return target_uri;
    } else {
        return uri;
    }
}

ComputerRemoteVolumeItem::ComputerRemoteVolumeItem(const QString &uri, ComputerModel *model, AbstractComputerItem *parentNode, QObject *parent) : AbstractComputerItem(model, parentNode, parent)
{
    m_uri = uri;
    m_cancellable = g_cancellable_new();
    updateInfo();

    auto targetUri = queryTargetUri(uri);
    m_model->m_volumeTargetMap.insert(uri, targetUri);
    m_model->addRealUri(targetUri);

    bool isKydrive = targetUri.startsWith("file://") && targetUri.contains("kydrive");
    setProperty("isKydrive", isKydrive);

    if (uri == "computer:///") {
        m_isHidden = false;
    } else {
        //fix dock show in remote item issue, link to bug#82398
        if(targetUri.startsWith("ftp://") || targetUri.startsWith("sftp://") || targetUri.startsWith("smb://") || targetUri.startsWith("file:///"))
            m_isHidden = false;
        else
            m_isHidden = true;
    }

    qDebug()<<"create remote volume item:"<<uri;
}

ComputerRemoteVolumeItem::~ComputerRemoteVolumeItem()
{
    if (m_cancellable) {
        g_cancellable_cancel(m_cancellable);
        g_object_unref(m_cancellable);
    }

    m_model->m_volumeTargetMap.remove(m_uri);
    m_model->removeRealUri(m_uri);
}

const QString ComputerRemoteVolumeItem::displayName()
{
    if (m_uri == "computer:///")
        return tr("Remote");
    return m_displayName;
}

const QIcon ComputerRemoteVolumeItem::icon()
{
    return m_icon;
}

void ComputerRemoteVolumeItem::findChildren()
{
    if (m_uri != "computer:///")
        return;

    GFile *file = g_file_new_for_uri("computer:///");
    g_file_enumerate_children_async(file, G_FILE_ATTRIBUTE_STANDARD_NAME, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, 0,
                                    m_cancellable, GAsyncReadyCallback(enumerate_async_callback), this);
    g_object_unref(file);


}

void ComputerRemoteVolumeItem::updateInfo()
{
    GFile *file = g_file_new_for_uri(m_uri.toUtf8().constData());
    g_file_query_info_async(file, "*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, 0,
                            m_cancellable, GAsyncReadyCallback(query_info_async_callback), this);
    g_object_unref(file);
}

void ComputerRemoteVolumeItem::unmount(GMountUnmountFlags unmountFlag)
{
    QString targetUri = queryTargetUri(m_uri);
    GFile *file = g_file_new_for_uri(targetUri.toUtf8().constData());
    //g_file_unmount_mountable_with_operation(file, unmountFlag, nullptr, nullptr, nullptr, nullptr);
    GMount *gMount = g_file_find_enclosing_mount(file, nullptr, nullptr);
    g_mount_unmount_with_operation(gMount,
                    G_MOUNT_UNMOUNT_NONE,
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr);
    g_object_unref(file);
    g_object_unref(gMount);
}

QModelIndex ComputerRemoteVolumeItem::itemIndex()
{
    if (!m_parentNode)
        return m_model->createItemIndex(0, this);

    return m_model->createItemIndex(m_parentNode->m_children.indexOf(this), this);
}

bool ComputerRemoteVolumeItem::isHidden()
{
//  return m_isUnixDevice || m_uri == "computer:///root.link";
    //fix bug#82398, hide unknow devices
    return m_isHidden;
}

void ComputerRemoteVolumeItem::onFileAdded(const QString &uri)
{
    if (!uri.endsWith(".mount")) {
        return;
    }
    //not include udisk、mobile-disk、local-partition etc.
    QString targetUri;
    targetUri = queryTargetUri(uri);
    m_model->m_volumeTargetMap.insert(uri, targetUri);

    m_model->addRealUri(uri);

//    if(!targetUri.isEmpty() && targetUri.contains("file:///"))
//        return;

    for (auto item : m_children) {
        //qDebug() << "ComputerRemoteVolumeItem onFileAdded uri:"<<uri<<item->uri();
        if (item->uri() == uri)
            return;
    }

    m_model->beginInsertItem(itemIndex(), m_children.count());
    auto item = new ComputerRemoteVolumeItem(uri, m_model, this);
    m_children<<item;
    m_model->endInsterItem();

    m_model->updateRequest();
    m_model->invalidateRequest();
}

void ComputerRemoteVolumeItem::onFileRemoved(const QString &uri)
{
    int row = -1;
    for (auto item : m_children) {
        //qDebug() << "ComputerRemoteVolumeItem onFileRemoved uri:"<<uri<<item->uri();
        if (item->uri() == uri) {
            row = m_children.indexOf(item);
            break;
        }
    }

    if (row < 0)
        return;

    m_model->beginRemoveItem(itemIndex(), row);
    auto item = m_children.takeAt(row);
    item->deleteLater();
    m_model->endRemoveItem();

    m_model->updateRequest();
    m_model->invalidateRequest();
}

void ComputerRemoteVolumeItem::onFileChanged(const QString &uri)
{
    for (auto item : m_children) {
        if (item->uri() == uri) {
            item->updateInfo();
            return;
        }
    }
}

void ComputerRemoteVolumeItem::enumerate_async_callback(GFile *file, GAsyncResult *res, ComputerRemoteVolumeItem *p_this)
{
    GError *err = nullptr;
    auto enumerator = g_file_enumerate_children_finish(file, res, &err);
    if (enumerator) {
        g_file_enumerator_next_files_async(enumerator, 9999, 0, p_this->m_cancellable,
                                           GAsyncReadyCallback(find_children_async_callback), p_this);
    }
    if (err) {
        p_this->m_isUnixDevice = true; //hide computer:///
        //QMessageBox::critical(0, 0, err->message);
        g_error_free(err);
    }
}

void ComputerRemoteVolumeItem::find_children_async_callback(GFileEnumerator *enumerator, GAsyncResult *res, ComputerRemoteVolumeItem *p_this)
{
    GError *err = nullptr;
    auto infos = g_file_enumerator_next_files_finish(enumerator, res, &err);
    GList *l = infos;
    while (l) {
        auto info = G_FILE_INFO(l->data);
        l = l->next;
        if (!info)
            continue;

        auto file = g_file_enumerator_get_child(enumerator, info);
        if (!file)
            continue;

        auto uri = g_file_get_uri(file);
        if (!uri)
            continue;

        QString tmp = uri;
        if (!tmp.endsWith(".mount")) {
            continue;
        }

        //not include udisk、mobile-disk、local-partition etc.
        QString targetUri;
        targetUri = queryTargetUri(uri);
        p_this->m_model->m_volumeTargetMap.insert(uri, targetUri);
        p_this->m_model->addRealUri(uri);

        p_this->m_model->beginInsertItem(p_this->itemIndex(), p_this->m_children.count());
        auto item = new ComputerRemoteVolumeItem(uri, p_this->m_model, p_this);
        p_this->m_children<<item;
        p_this->m_model->endInsterItem();

        p_this->m_model->updateRequest();
        g_free(uri);
        g_object_unref(file);
    }

    if (infos)
        g_list_free_full(infos, g_object_unref);

    if (enumerator) {
        g_file_enumerator_close(enumerator, nullptr, nullptr);
        g_object_unref(enumerator);
    }

    if (err) {
        //QMessageBox::critical(0, 0, err->message);
        g_error_free(err);
    } else {
        p_this->m_watcher = new Peony::FileWatcher("computer:///", p_this);
        connect(p_this->m_watcher, &Peony::FileWatcher::fileCreated, p_this, &ComputerRemoteVolumeItem::onFileAdded);
        connect(p_this->m_watcher, &Peony::FileWatcher::fileDeleted, p_this, &ComputerRemoteVolumeItem::onFileRemoved);
        connect(p_this->m_watcher, &Peony::FileWatcher::fileChanged, p_this, &ComputerRemoteVolumeItem::onFileChanged);
        p_this->m_watcher->startMonitor();
    }
}

void ComputerRemoteVolumeItem::query_info_async_callback(GFile *file, GAsyncResult *res, ComputerRemoteVolumeItem *p_this)
{
    GError *err = nullptr;
    GFileInfo *info = g_file_query_info_finish(file, res, &err);
    if (info) {
        p_this->m_isUnixDevice = g_file_info_has_attribute(info, G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE_FILE);
        p_this->m_displayName = g_file_info_get_attribute_string(info, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);
        GIcon *icon = g_file_info_get_icon(info);
        const gchar * const * names = g_themed_icon_get_names(G_THEMED_ICON(icon));
        if (names) {
            const char *name = *names;
            if (name) {
                p_this->m_icon = QIcon::fromTheme(name);
            }
        }

        p_this->m_model->dataChanged(p_this->itemIndex(), p_this->itemIndex());
        qDebug()<<"query_info_async_callback:"<<p_this->m_uri<<p_this->m_isUnixDevice;

        g_object_unref(info);
    }

    if (err) {
        //QMessageBox::critical(0, 0, err->message);
        g_error_free(err);
    }
}
