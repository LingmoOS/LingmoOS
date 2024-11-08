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
 * Authors: Ding Jing <dingjing@kylinos.cn>
 *
 */

#ifndef FAVORITEVFSFILEMONITOR_H
#define FAVORITEVFSFILEMONITOR_H

#include <gio/gio.h>

#include <QObject>

G_BEGIN_DECLS

#define VFS_FAVORITE_FILE_MONITOR_NAME              ("vfs-favorite-file-monitor")

#define VFS_TYPE_FAVORITE_FILE_MONITOR              (vfs_favorite_file_monitor_get_type())
#define VFS_FAVORITE_FILE_MONITOR(o)                (G_TYPE_CHECK_INSTANCE_CAST ((o), VFS_TYPE_FAVORITE_FILE_MONITOR, FavoriteVFSFileMonitor))
#define VFS_FAVORITE_FILE_MONITOR_CLASS(k)          (G_TYPE_CLASS_CAST((k), VFS_TYPE_FAVORITE_FILE_MONITOR, FavoriteVFSFileMonitorClass))
#define VFS_IS_FAVORITE_FILE_MONITOR(o)             (G_TYPE_CHECK_INSTANCE_TYPE((o), VFS_TYPE_FAVORITE_FILE_MONITOR))
#define VFS_IS_FAVORITE_FILE_MONITOR_CLASS(k)       (G_TYPE_CHECK_CLASS_TYPE((k), VFS_TYPE_FAVORITE_FILE_MONITOR))
#define VFS_FAVORITE_FILE_MONITOR_GET_CLASS(o)      (G_TYPE_INSTANCE_GET_CLASS((o), VFS_TYPE_FAVORITE_FILE_MONITOR, FavoriteVFSFileMonitorClass))

typedef struct _FavoriteVFSFileMonitor              FavoriteVFSFileMonitor;
typedef struct _FavoriteVFSFileMonitorClass         FavoriteVFSFileMonitorClass;
typedef struct _FavoriteVFSFileMonitorSource        FavoriteVFSFileMonitorSource;
typedef struct _FavoriteVFSFileMonitorPrivate       FavoriteVFSFileMonitorPrivate;

struct _FavoriteVFSFileMonitor
{
    GFileMonitor                        parent_instance;

    FavoriteVFSFileMonitorPrivate*      priv;
    FavoriteVFSFileMonitorSource*       source;
    GList*                              fileList;
    QMetaObject::Connection             add;
    QMetaObject::Connection             remove;
};

struct _FavoriteVFSFileMonitorClass
{
    GFileMonitorClass parent_class;

    void (*start) (FavoriteVFSFileMonitor*, const gchar*, const gchar*, const gchar*, FavoriteVFSFileMonitorSource*);
};

GType                       vfs_favorite_file_monitor_get_type (void);
void                        vfs_favorite_file_monitor_dir (FavoriteVFSFileMonitor* obj);
void                        vfs_favorite_file_monitor_free_gfile (FavoriteVFSFileMonitor* obj, GFile*);
FavoriteVFSFileMonitor*     vfs_favorite_file_monitor_new_for_path (const char* path, gboolean isDirectory, GFileMonitorFlags flags, GError**);
gboolean                    vfs_favorite_file_monitor_source_handle_event (FavoriteVFSFileMonitorSource*, GFileMonitorEvent, const gchar* child, const gchar* renameTo, GFile* other, gint64 eTime);

G_END_DECLS
#endif // FAVORITEVFSFILEMONITOR_H
