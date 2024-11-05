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

#ifndef FILEVFSINFOPRIVATE_H
#define FILEVFSINFOPRIVATE_H

#include "explorer-core_global.h"
#include <gio/gio.h>
#include <QMap>
#include <QVariant>
#include <QObject>

namespace Peony {
class FileVFSInfo;
class PEONYCORESHARED_EXPORT FileVFSInfoPrivate : QObject
{
    Q_OBJECT
public:
    explicit FileVFSInfoPrivate(FileVFSInfo* parent);
    ~FileVFSInfoPrivate();

    FileVFSInfo *const qptr;

public:
    QString targetUri() const;
    void setTargetUri(const QString &targetUri);

    bool isDir() const;
    void setDir(const bool &dir);

    bool isVirtual() const;
    void setVirtual(const bool &isVirtual);

    QString iconName() const;
    void setIconName(const QString &iconName);

    QString filePath() const;
    void setFilePath(const QString &path);

    QString contentType() const;
    void setContentType(const QString &contentType);

    quint64 size() const;
    void setSize(const quint64 &size);

    QString modifiedTime() const;
    void setModifiedTime(const QString &modifiedTime);

    QString accessTime() const;
    void setAccessTime(const QString &accessTime);

    QString deletionTime() const;
    void setDeletionTime(const QString &deletionTime);

    bool canRead() const;
    void setCanRead(const bool &canRead);

    bool canWrite() const;
    void setCanWrite(const bool &canWrite);

    bool canExecute() const;
    void setCanExecute(const bool &canExecute);

    bool canDelete() const;
    void setCanDelete(const bool &canDelete);

    bool canTrash() const;
    void setCanTrash(const bool &canTrash);

    bool canRename() const;
    void setCanRename(const bool &canRename);

    void addExtendInfo(const QString &key, const QVariant &value);
    void removeExtendInfo(const QString &key);
    QVariant getExtendInfo(const QString &key) const;
    QMap<QString, QVariant> getAllExtendInfo() const;


    QString m_uri = nullptr;
    QString m_target_uri = nullptr;
    bool m_is_dir = false;
    bool m_is_virtual = false;

    QString m_display_name = nullptr;
    QString m_icon_name = nullptr;
    QString m_path = nullptr;
    QString m_contentType = nullptr;
    guint64 m_size = 0;
    QString m_modified_time = nullptr;
    QString m_access_time = nullptr;
    QString m_deletion_data = nullptr;

    bool m_can_read = true;
    bool m_can_write = false;
    bool m_can_excute = false;
    bool m_can_delete = false;
    bool m_can_trash = false;
    bool m_can_rename = false;

    QMap<QString, QVariant> m_extend_info;

};
}

#endif // FILEVFSINFOPRIVATE_H
