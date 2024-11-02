/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 */

#include "custom-vfs-file-monitor.h"
#include "custom-vfs-file.h"
#include "vfs-info-plugin-iface.h"
#include "vfs-info-plugin-manager.h"
#include "file-utils.h"
#include <QDebug>

G_DEFINE_TYPE(CustomVFSFileMonitor, vfs_custom_file_monitor, G_TYPE_FILE_MONITOR)

static void vfs_custom_file_monitor_dispose(GObject *obj);
static void vfs_custom_file_monitor_finalize(GObject *obj);

static void vfs_custom_file_monitor_init(CustomVFSFileMonitor *self) {
    g_return_if_fail(VFS_IS_CUSTOM_FILE_MONITOR(self));
}

static void vfs_custom_file_monitor_class_init(CustomVFSFileMonitorClass *klass) {
    GObjectClass* selfClass = G_OBJECT_CLASS(klass);

    selfClass->dispose = vfs_custom_file_monitor_dispose;
    selfClass->finalize = vfs_custom_file_monitor_finalize;
}

static void vfs_custom_file_monitor_dispose(GObject *obj) {
    g_return_if_fail(VFS_IS_CUSTOM_FILE_MONITOR(obj));
    CustomVFSFileMonitor* self = VFS_CUSTOM_FILE_MONITOR(obj);

    QObject::disconnect(self->add);
    QObject::disconnect(self->remove);
    QObject::disconnect(self->change);
}

static void vfs_custom_file_monitor_finalize(GObject *obj) {
    g_return_if_fail(VFS_IS_CUSTOM_FILE_MONITOR(obj));
    G_OBJECT_CLASS(vfs_custom_file_monitor_parent_class)->finalize(obj);
}

static CustomVFSFileMonitor *vfs_custom_file_monitor_new() {
    return VFS_CUSTOM_FILE_MONITOR(g_object_new(VFS_TYPE_CUSTOM_FILE_MONITOR, nullptr));
}

void vfs_custom_file_monitor_dir(CustomVFSFileMonitor *obj, const QString &filepath) {
    g_return_if_fail(VFS_IS_CUSTOM_FILE_MONITOR(obj));

    QString strUri = filepath;
    QString scheme = strUri.section(":", 0, -2);

    Peony::VFSInfoPluginIface *iface = Peony::VFSInfoPluginManager::getInstance()->userSchemeGetPlugin(scheme);
    Peony::HanderTransfer *transfer = new Peony::HanderTransfer();
    if (iface) {
        iface->handerMonitorDirectory(transfer, filepath);
    }

    obj->add = QObject::connect(transfer, &Peony::HanderTransfer::fileCreate, [=](const QString &path){
        qDebug() << __func__ << "fileCreate:" << path;
        if (!path.isEmpty()) {
            QString tScheme = scheme + "://";
            QString realUri = tScheme + path;
            g_autoptr(GFile) file = g_file_new_for_uri(realUri.toUtf8().constData());

            g_file_monitor_emit_event(G_FILE_MONITOR(obj), file, nullptr, G_FILE_MONITOR_EVENT_CREATED);
        }
    });

    obj->remove = QObject::connect(transfer, &Peony::HanderTransfer::fileDelete, [=](const QString &path){
        qDebug() << __func__ << "fileDelete:" << path;
        if (!path.isEmpty()) {
            QString tScheme = scheme + "://";
            QString realUri = tScheme + path;
            g_autoptr(GFile) file = g_file_new_for_uri(realUri.toUtf8().constData());

            g_file_monitor_emit_event(G_FILE_MONITOR(obj), file, nullptr, G_FILE_MONITOR_EVENT_DELETED);
        }
    });

    obj->change = QObject::connect(transfer, &Peony::HanderTransfer::fileChanged, [=](const QString &path){
        qDebug() << __func__ << "fileChanged:" << path;
        if (!path.isEmpty()) {
            QString tScheme = scheme + "://";
            QString realUri = tScheme + path;
            g_autoptr(GFile) file = g_file_new_for_uri(realUri.toUtf8().constData());

            g_file_monitor_emit_event(G_FILE_MONITOR(obj), file, nullptr, G_FILE_MONITOR_EVENT_CHANGED);
        }
    });
}
