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

#include "file-vfs-info-private.h"

using namespace Peony;

FileVFSInfoPrivate::FileVFSInfoPrivate(FileVFSInfo *parent)
    : qptr(parent)
{

}

FileVFSInfoPrivate::~FileVFSInfoPrivate()
{

}

QString FileVFSInfoPrivate::targetUri() const
{
    return m_target_uri;
}

void FileVFSInfoPrivate::setTargetUri(const QString &targetUri)
{
    m_target_uri = targetUri;
}

bool FileVFSInfoPrivate::isDir() const
{
    return m_is_dir;
}

void FileVFSInfoPrivate::setDir(const bool &dir)
{
    m_is_dir = dir;
}

bool FileVFSInfoPrivate::isVirtual() const
{
    return m_is_virtual;
}

void FileVFSInfoPrivate::setVirtual(const bool &isVirtual)
{
    m_is_virtual = isVirtual;
}

QString FileVFSInfoPrivate::iconName() const
{
    return m_icon_name;
}

void FileVFSInfoPrivate::setIconName(const QString &iconName)
{
    m_icon_name = iconName;
}

QString FileVFSInfoPrivate::filePath() const
{
    return m_path;
}

void FileVFSInfoPrivate::setFilePath(const QString &path)
{
    m_path = path;
}

QString FileVFSInfoPrivate::contentType() const
{
    return m_contentType;
}

void FileVFSInfoPrivate::setContentType(const QString &contentType)
{
    m_contentType = contentType;
}

quint64 FileVFSInfoPrivate::size() const
{
    return m_size;
}

void FileVFSInfoPrivate::setSize(const quint64 &size)
{
    m_size = size;
}

QString FileVFSInfoPrivate::modifiedTime() const
{
    return m_modified_time;
}

void FileVFSInfoPrivate::setModifiedTime(const QString &modifiedTime)
{
    m_modified_time = modifiedTime;
}

QString FileVFSInfoPrivate::accessTime() const
{
    return m_access_time;
}

void FileVFSInfoPrivate::setAccessTime(const QString &accessTime)
{
    m_access_time = accessTime;
}

QString FileVFSInfoPrivate::deletionTime() const
{
    return m_deletion_data;
}

void FileVFSInfoPrivate::setDeletionTime(const QString &deletionTime)
{
    m_deletion_data = deletionTime;
}

bool FileVFSInfoPrivate::canRead() const
{
    return m_can_read;
}

void FileVFSInfoPrivate::setCanRead(const bool &canRead)
{
    m_can_read = canRead;
}

bool FileVFSInfoPrivate::canWrite() const
{
    return m_can_write;
}

void FileVFSInfoPrivate::setCanWrite(const bool &canWrite)
{
    m_can_write = canWrite;
}

bool FileVFSInfoPrivate::canExecute() const
{
    return m_can_excute;
}

void FileVFSInfoPrivate::setCanExecute(const bool &canExecute)
{
    m_can_excute = canExecute;
}

bool FileVFSInfoPrivate::canDelete() const
{
    return m_can_delete;
}

void FileVFSInfoPrivate::setCanDelete(const bool &canDelete)
{
    m_can_delete = canDelete;
}

bool FileVFSInfoPrivate::canTrash() const
{
    return m_can_trash;
}

void FileVFSInfoPrivate::setCanTrash(const bool &canTrash)
{
    m_can_trash = canTrash;
}

bool FileVFSInfoPrivate::canRename() const
{
    return m_can_rename;
}

void FileVFSInfoPrivate::setCanRename(const bool &canRename)
{
    m_can_rename = canRename;
}

void FileVFSInfoPrivate::addExtendInfo(const QString &key, const QVariant &value)
{
    m_extend_info.insert(key, value);
}

void FileVFSInfoPrivate::removeExtendInfo(const QString &key)
{
    m_extend_info.remove(key);
}

QVariant FileVFSInfoPrivate::getExtendInfo(const QString &key) const
{
    if (key.isEmpty()) {
        return QVariant();
    } else {
        return m_extend_info.value(key);
    }
}

QMap<QString, QVariant> FileVFSInfoPrivate::getAllExtendInfo() const
{
    return m_extend_info;
}
