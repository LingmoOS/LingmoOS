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
 * Authors: Ling Yang <yangling@kylinos.cn>
 *
 */

#ifndef LABELVFSFILEMONITOR_H
#define LABELVFSFILEMONITOR_H

#include <gio/gio.h>
#include <QObject>


G_BEGIN_DECLS

#define VFS_LABEL_FILE_MONITOR_NAME         ("vfs-label-file-monitor")

#define VFS_TYPE_LABEL_FILE_MONITOR         (vfs_label_file_monitor_get_type ())
#define VFS_LABEL_FILE_MONITOR(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), VFS_TYPE_LABEL_FILE_MONITOR, LabelVFSFileMonitor))
#define VFS_LABEL_FILE_MONITOR_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), VFS_TYPE_LABEL_FILE_MONITOR, LabelVFSFileMonitorClass))
#define VFS_IS_LABEL_FILE_MONITOR(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), VFS_TYPE_LABEL_FILE_MONITOR))
#define VFS_IS_LABEL_FILE_MONITOR_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), VFS_TYPE_LABEL_FILE_MONITOR))
#define VFS_LABEL_FILE_MONITOR_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), VFS_TYPE_LABEL_FILE_MONITOR, LabelVFSFileMonitorClass))

/**
 * GFileMonitor:
 *
 * Watches for changes to a file.
 **/

struct LabelVFSFileMonitorPrivate
{
    //GList* fileList = nullptr;
    gchar*                       label_vfs_directory_uri;
};

struct LabelVFSFileMonitor
{
    GFileMonitor                 parent_instance;
    LabelVFSFileMonitorPrivate   *priv = nullptr;
    GList                        *fileList = nullptr;
    QMetaObject::Connection      add;
    QMetaObject::Connection      remove;
};

struct LabelVFSFileMonitorClass
{
    GFileMonitorClass  parent_class;
};


GType  vfs_label_file_monitor_get_type  (void) G_GNUC_CONST;
void   vfs_label_file_monitor_dir(LabelVFSFileMonitor *obj, const QString &label_vfs_directory_uri);


G_END_DECLS

#endif // LABELVFSFILEMONITOR_H
