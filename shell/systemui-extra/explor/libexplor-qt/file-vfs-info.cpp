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

#include "file-vfs-info.h"
#include "private/file-vfs-info-private.h"

using namespace Peony;

FileVFSInfo::FileVFSInfo(QObject *parent)
    : QObject(parent), dptr(new FileVFSInfoPrivate(this))
{

}

FileVFSInfo::~FileVFSInfo()
{

}

QString FileVFSInfo::targetUri() const
{
    return dptr->targetUri();
}

void FileVFSInfo::setTargetUri(const QString &targetUri)
{
    dptr->setTargetUri(targetUri);
}

bool FileVFSInfo::isDir() const
{
    return dptr->isDir();
}

void FileVFSInfo::setDir(const bool &dir)
{
    dptr->setDir(dir);
}

bool FileVFSInfo::isVirtual() const
{
    return dptr->isVirtual();
}

void FileVFSInfo::setVirtual(const bool &isVirtual)
{
    dptr->setVirtual(isVirtual);
}

QString FileVFSInfo::iconName() const
{
    return dptr->iconName();
}

void FileVFSInfo::setIconName(const QString &iconName)
{
    dptr->setIconName(iconName);
}

QString FileVFSInfo::filePath() const
{
    return dptr->filePath();
}

void FileVFSInfo::setFilePath(const QString &path)
{
    dptr->setFilePath(path);
}

QString FileVFSInfo::contentType() const
{
    return dptr->contentType();
}

void FileVFSInfo::setContentType(const QString &contentType)
{
    dptr->setContentType(contentType);
}

quint64 FileVFSInfo::size() const
{
    return dptr->size();
}

void FileVFSInfo::setSize(const quint64 &size)
{
    dptr->setSize(size);
}

QString FileVFSInfo::modifiedTime() const
{
    return dptr->modifiedTime();
}

void FileVFSInfo::setModifiedTime(const QString &modifiedTime)
{
    dptr->setModifiedTime(modifiedTime);
}

QString FileVFSInfo::accessTime() const
{
    return dptr->accessTime();
}

void FileVFSInfo::setAccessTime(const QString &accessTime)
{
    dptr->setAccessTime(accessTime);
}

QString FileVFSInfo::deletionTime() const
{
    return dptr->deletionTime();
}

void FileVFSInfo::setDeletionTime(const QString &deletionTime)
{
    dptr->setDeletionTime(deletionTime);
}

bool FileVFSInfo::canRead() const
{
    return dptr->canRead();
}

void FileVFSInfo::setCanRead(const bool &canRead)
{
    dptr->setCanRead(canRead);
}

bool FileVFSInfo::canWrite() const
{
    return dptr->canWrite();
}

void FileVFSInfo::setCanWrite(const bool &canWrite)
{
    dptr->setCanWrite(canWrite);
}

bool FileVFSInfo::canExecute() const
{
    return dptr->canExecute();
}

void FileVFSInfo::setCanExecute(const bool &canExecute)
{
    dptr->setCanExecute(canExecute);
}

bool FileVFSInfo::canDelete() const
{
    return dptr->canDelete();
}

void FileVFSInfo::setCanDelete(const bool &canDelete)
{
    dptr->setCanDelete(canDelete);
}

bool FileVFSInfo::canTrash() const
{
    return dptr->canTrash();
}

void FileVFSInfo::setCanTrash(const bool &canTrash)
{
    dptr->setCanTrash(canTrash);
}

bool FileVFSInfo::canRename() const
{
    return dptr->canRename();
}

void FileVFSInfo::setCanRename(const bool &canRename)
{
    dptr->setCanRename(canRename);
}

void FileVFSInfo::addExtendInfo(const QString &key, const QVariant &value)
{
    dptr->addExtendInfo(key, value);
}

void FileVFSInfo::removeExtendInfo(const QString &key)
{
    dptr->removeExtendInfo(key);
}

QVariant FileVFSInfo::getExtendInfo(const QString &key) const
{
    if (key.isEmpty()) {
        return QVariant();
    } else {
        return dptr->getExtendInfo(key);
    }
}

QMap<QString, QVariant> FileVFSInfo::getAllExtendInfo() const
{
    return dptr->getAllExtendInfo();
}
