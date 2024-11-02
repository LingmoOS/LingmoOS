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

#ifndef TESTVFSFILEENUMERATOR_H
#define TESTVFSFILEENUMERATOR_H

#include <gio/gio.h>

#include <QQueue>

G_BEGIN_DECLS

#define VFS_TYPE_CUSTOM_FILE_ENUMERATOR vfs_custom_file_enumerator_get_type()

G_DECLARE_FINAL_TYPE(CustomVFSFileEnumerator, vfs_custom_file_enumerator, VFS, CUSTOM_FILE_ENUMERATOR, GFileEnumerator)

CustomVFSFileEnumerator* vfs_custom_file_enumerator_new(void);

typedef struct _CustomVFSFileEnumeratorPrivate       CustomVFSFileEnumeratorPrivate;

struct _CustomVFSFileEnumeratorPrivate
{
    QQueue<QString>*                    enumerate_queue;
};

struct _CustomVFSFileEnumerator
{
    GFileEnumerator                     parent_instance;

    CustomVFSFileEnumeratorPrivate*  priv;
};

G_END_DECLS

#endif // TESTVFSFILEENUMERATOR_H
