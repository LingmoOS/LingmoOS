/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BALOO_ID_UTILS_
#define BALOO_ID_UTILS_

#include <qplatformdefs.h>
#include <qglobal.h>

#ifdef Q_OS_WIN
# include <QFileInfo>
#else
# include <sys/statvfs.h>
#endif

namespace Baloo {

inline quint64 devIdAndInodeToId(quint32 devId, quint32 inode)
{
    quint64 res;
    quint32 arr[2];
    arr[0] = devId;
    arr[1] = inode;

    memcpy(&res, arr, sizeof(arr));
    return res;
}

/**
 * Convert the QT_STATBUF into a 64 bit unique identifier for the file.
 * This identifier is combination of the device id and inode number.
 */
inline quint64 statBufToId(const QT_STATBUF& stBuf)
{
    // We're losing 32 bits of info, so this could potentially break
    // on file systems with really large inode and device ids
    return devIdAndInodeToId(static_cast<quint32>(stBuf.st_dev),
                             static_cast<quint32>(stBuf.st_ino));
}

#ifndef Q_OS_WIN
inline int statWithFsid(const char* path, QT_STATBUF* statBuf)
{
    int ret = QT_LSTAT(path, statBuf);
    if (ret != 0) {
        return ret;
    }

    struct statvfs fsBuf;
    ret = statvfs(path, &fsBuf);
    if (ret == 0 && fsBuf.f_fsid != 0) {
        // Fold FSID into 32 bits, statBufToId would discard anything else
        statBuf->st_dev = static_cast<quint32>(fsBuf.f_fsid ^ (fsBuf.f_fsid >> 32));
    }
    return ret;
}
#endif

inline int filePathToStat(const QByteArray& filePath, QT_STATBUF& statBuf)
{
#ifndef Q_OS_WIN
    return statWithFsid(filePath.constData(), &statBuf);
#else
    const int ret = QT_STAT(filePath.constData(), &statBuf);
    const QString filePathStr = QString::fromUtf8(filePath);
    if (ret == 0 && QFileInfo(filePathStr).isSymLink()) {
        return QT_STAT(QFileInfo(filePathStr).symLinkTarget().toUtf8().constData(), &statBuf);
    } else {
        return ret;
    }
#endif
}

inline quint64 filePathToId(const QByteArray& filePath)
{
    QT_STATBUF statBuf;
    const int ret = filePathToStat(filePath, statBuf);
    return ret ? 0 : statBufToId(statBuf);
}

inline quint32 idToInode(quint64 id)
{
    quint32* arr = reinterpret_cast<quint32*>(&id);
    return arr[1];
}

inline quint32 idToDeviceId(quint64 id)
{
    quint32* arr = reinterpret_cast<quint32*>(&id);
    return arr[0];
}

template<typename T, typename V>
inline void sortedIdInsert(T& vec, const V& id)
{
    /**
     * search with normal <
     */
    const auto i(std::lower_bound(vec.begin(), vec.end(), id));

    /**
     * end reached or element found smaller?
     * => insert new element!
     */
    if (i == vec.end() || (id != *i))
        vec.insert(i, id);
}

template<typename T, typename V>
inline void sortedIdRemove(T& vec, const V& id)
{
    const int idx = vec.indexOf(id);
    if (idx >= 0) {
        vec.remove(idx);
    }
}

}

#endif
