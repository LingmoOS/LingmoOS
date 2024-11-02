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

#ifndef LABELVFSFILE_H
#define LABELVFSFILE_H

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define VFS_TYPE_LABEL_FILE                      (vfs_label_file_get_type())

#define VFS_IS_LABEL_FILE_CLASS(k)               (G_TYPE_CHECK_CLASS_TYPE((k), VFS_TYPE_LABEL_FILE))
#define VFS_IS_FAVORITE_FILE(o)                  (G_TYPE_CHECK_INSTANCE_TYPE((o), VFS_TYPE_LABEL_FILE))
#define VFS_LABEL_FILE_CLASS(k)                  (G_TYPE_CLASS_CAST((k), VFS_TYPE_LABEL_FILE, LabelVFSFileClass))
//#define VFS_LABEL_FILE(o)                        (G_TYPE_CHECK_INSTANCE_CAST((o), VFS_TYPE_LABEL_FILE, LabelVFSFile))
#define VFS_LABEL_FILE_GET_CLASS(o)              (G_TYPE_INSTANCE_GET_CLASS((o), VFS_TYPE_LABEL_FILE, LabelVFSFileClass))

G_DECLARE_FINAL_TYPE(LabelVFSFile, vfs_label_file, VFS, LABEL_FILE, GObject)

LabelVFSFile* vfs_label_file_new(void);

struct LabelVFSFilePrivate
{
    gchar*                      uri;
    GFileMonitor*               fileMonitor;
};

struct _LabelVFSFile
{
    GObject                     parent_instance;
    LabelVFSFilePrivate*        priv;
};

GFile*   vfs_label_file_new_for_uri(const char *uri);
gboolean vfs_label_file_is_exist (const char* uri);

G_END_DECLS

#endif // LABELVFSFILE_H
