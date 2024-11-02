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

#ifndef LABELVFSFILEENUMERATOR_H
#define LABELVFSFILEENUMERATOR_H

#include <gio/gio.h>
#include <QQueue>

G_BEGIN_DECLS

#define VFS_TYPE_LABEL_FILE_ENUMERATOR vfs_label_file_enumerator_get_type()

G_DECLARE_FINAL_TYPE(LabelVFSFileEnumerator, vfs_label_file_enumerator, VFS, LABEL_FILE_ENUMERATOR, GFileEnumerator)

LabelVFSFileEnumerator* vfs_label_file_enumerator_new(void);

struct LabelVFSFileEnumeratorPrivate
{
    QString*         label_vfs_directory_uri = nullptr;
    QQueue<QString>* enumerate_queue = nullptr;
};

struct _LabelVFSFileEnumerator
{
    GFileEnumerator parent_instance;

    LabelVFSFileEnumeratorPrivate*  priv = nullptr;
};

G_END_DECLS

#endif // LABELVFSFILEENUMERATOR_H
