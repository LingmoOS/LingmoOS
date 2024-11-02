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

#ifndef TESTVFSFILEMONITOR_H
#define TESTVFSFILEMONITOR_H

#include <gio/gio.h>
#include <QObject>

G_BEGIN_DECLS

#define VFS_TYPE_CUSTOM_FILE_MONITOR (vfs_custom_file_monitor_get_type())
#define VFS_CUSTOM_FILE_MONITOR(o) (G_TYPE_CHECK_INSTANCE_CAST((o), VFS_TYPE_CUSTOM_FILE_MONITOR, CustomVFSFileMonitor))
#define VFS_CUSTOM_FILE_MONITOR_CLASS(k) (G_TYPE_CLASS_CAST((k), VFS_TYPE_CUSTOM_FILE_MONITOR, CustomVFSFileMonitorClass))
#define VFS_IS_CUSTOM_FILE_MONITOR(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), VFS_TYPE_CUSTOM_FILE_MONITOR))
#define VFS_IS_CUSTOM_FILE_MONITOR_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k), VFS_TYPE_CUSTOM_FILE_MONITOR))
#define VFS_CUSTOM_FILE_MONITOR_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS(o, VFS_TYPE_CUSTOM_FILE_MONITOR, CustomVFSFileMonitorClass))

typedef struct _CustomVFSFileMonitor CustomVFSFileMonitor;
typedef struct _CustomVFSFileMonitorClass CustomVFSFileMonitorClass;

struct _CustomVFSFileMonitor
{
    GFileMonitor parent_monitor;
    QMetaObject::Connection add;
    QMetaObject::Connection remove;
    QMetaObject::Connection change;
};

struct _CustomVFSFileMonitorClass {
    GFileMonitorClass parent_class;
};

GType vfs_custom_file_monitor_get_type(void);
void vfs_custom_file_monitor_dir(CustomVFSFileMonitor *obj, const QString &filepath);

G_END_DECLS

#endif // TESTVFSFILEMONITOR_H
