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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef FILEMETAINFO_H
#define FILEMETAINFO_H

#include <QString>
#include <QStringList>
#include <QHash>
#include <QVariant>
#include <QMutex>

#include <memory>
#include <gio/gio.h>
#include <explorer-core_global.h>

namespace Peony {

/*!
 * \brief The FileMetaInfo class
 * \details
 * This class represent a data set abstracted from gvfs metadata.
 *
 * When a FileInfoJob queryed, a FileMetaInfo will be created for the FileInfo instance
 * which FileInfoJob hold. FileMetaInfo constructed with the GFileInfo handle the file query
 * job executed, and read all "metadata::" namespace datas then put them into a hash table.
 *
 * \note
 * You can use FileInfoMeta::fromUri(uri) to get a file's meta data in global, but you should make
 * sure that file's FileInfo is queryed yet.
 */
class PEONYCORESHARED_EXPORT FileMetaInfo
{
    friend class FileInfo;
    friend class FileInfoJob;
    friend class FileInfoManager;
public:
    static std::shared_ptr<FileMetaInfo> fromGFileInfo(const QString &uri, GFileInfo *g_info);
    static std::shared_ptr<FileMetaInfo> fromUri(const QString &uri);
    static std::shared_ptr<FileMetaInfo> dupFromUri(const QString &uri);

    FileMetaInfo(const QString &uri, GFileInfo *g_info);
    FileMetaInfo(const FileMetaInfo &other);
    FileMetaInfo(FileMetaInfo *other);
    ~FileMetaInfo();

    void setMetaInfoString(const QString &key, const QString &value);
    const QString getMetaInfoString(const QString &key);
    void setMetaInfoStringList(const QString &key, const QStringList &value);
    const QStringList getMetaInfoStringList(const QString &key);
    void setMetaInfoInt(const QString &key, int value);
    int getMetaInfoInt(const QString &key);
    void setMetaInfoVariant(const QString &key, const QVariant &value, bool syncToFile = true);
    const QVariant getMetaInfoVariant(const QString &key);

    void setMetaInfoStringListV1(const QString &key, const QStringList &value);
    const QStringList getMetaInfoStringListV1(const QString &key);

    void removeMetaInfo(const QString &key);

private:
    QString m_uri;
    QHash<QString, QVariant> m_meta_hash;
    QMutex m_mutex;
};

}

#endif // FILEMETAINFO_H
