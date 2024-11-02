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

#ifndef FAVORITEVFSFILE_H
#define FAVORITEVFSFILE_H

#include <QMetaObject>
#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define VFS_TYPE_FAVORITE_FILE                      (vfs_favorite_file_get_type())

#define VFS_IS_FAVORITE_FILE_CLASS(k)               (G_TYPE_CHECK_CLASS_TYPE((k), VFS_TYPE_FAVORITE_FILE))
#define VFS_IS_FAVORITE_FILE(o)                     (G_TYPE_CHECK_INSTANCE_TYPE((o), VFS_TYPE_FAVORITE_FILE))
#define VFS_FAVORITE_FILE_CLASS(k)                  (G_TYPE_CLASS_CAST((k), VFS_TYPE_FAVORITE_FILE, FavoriteVFSFileClass))
#define VFS_FAVORITE_FILE(o)                        (G_TYPE_CHECK_INSTANCE_CAST ((o), VFS_TYPE_FAVORITE_FILE, FavoriteVFSFile))
#define VFS_FAVORITE_FILE_GET_CLASS(o)              (G_TYPE_INSTANCE_GET_CLASS((o), VFS_TYPE_FAVORITE_FILE, FavoriteVFSFileClass))

G_DECLARE_FINAL_TYPE(FavoriteVFSFile, vfs_favorite_file, VFS, FAVORITES_FILE, GObject)

FavoriteVFSFile* vfs_favorite_file_new(void);

typedef struct _FavoriteVFSFilePrivate FavoriteVFSFilePrivate;

struct _FavoriteVFSFilePrivate
{
    gchar*                      uri;
    GFileMonitor*               fileMonitor;
};

struct _FavoriteVFSFile
{
    GObject                     parent_instance;
    FavoriteVFSFilePrivate*     priv;
};


GFile* vfs_favorite_file_new_for_uri (const char* uri);
gboolean vfs_favorite_file_is_exist (const char* uri);
static GFileEnumerator* vfs_favorite_file_enumerate_children_internal (GFile* file, const char* attribute, GFileQueryInfoFlags flags, GCancellable* cancellable, GError** error);

G_END_DECLS

#endif // FAVORITEVFSFILE_H
