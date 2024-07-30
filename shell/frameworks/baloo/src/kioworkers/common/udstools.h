/*
    SPDX-FileCopyrightText: 2005 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2007 Thiago Macieira <thiago@kde.org>
    SPDX-FileCopyrightText: 2020 Stefan Brüns <bruns@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BALOO_KIO_COMMON_UDSTOOLS_H_
#define BALOO_KIO_COMMON_UDSTOOLS_H_

#include "usergroupcache.h"
#include "idutils.h"
#include <QUrl>

namespace Baloo {

class UdsFactory
{
public:
    KIO::UDSEntry createUdsEntry(const QString& filePath) const;

private:
    UserGroupCache m_userGroupCache;
};

inline KIO::UDSEntry UdsFactory::createUdsEntry(const QString& filePath) const
{
    KIO::UDSEntry uds;

    QT_STATBUF statBuf;
    const QByteArray ba = QFile::encodeName(filePath);
    if (filePathToStat(ba, statBuf) != 0) {
	return uds;
    }

    uds.reserve(12);
    uds.fastInsert(KIO::UDSEntry::UDS_DEVICE_ID, statBuf.st_dev);
    uds.fastInsert(KIO::UDSEntry::UDS_INODE, statBuf.st_ino);

    mode_t type = statBuf.st_mode & S_IFMT;
    mode_t access = statBuf.st_mode & 07777;

    uds.fastInsert(KIO::UDSEntry::UDS_SIZE, statBuf.st_size);
    uds.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, type);
    uds.fastInsert(KIO::UDSEntry::UDS_ACCESS, access);
    uds.fastInsert(KIO::UDSEntry::UDS_MODIFICATION_TIME, statBuf.st_mtime);
    uds.fastInsert(KIO::UDSEntry::UDS_ACCESS_TIME, statBuf.st_atime);
#ifndef Q_OS_WIN
    uds.fastInsert(KIO::UDSEntry::UDS_USER, m_userGroupCache.getUserName(KUserId(statBuf.st_uid)));
    uds.fastInsert(KIO::UDSEntry::UDS_GROUP, m_userGroupCache.getGroupName(KGroupId(statBuf.st_gid)));
#else
#pragma message("TODO: st_uid and st_gid are always zero, use GetSecurityInfo to find the owner")
#endif

    QUrl url = QUrl::fromLocalFile(filePath);
    uds.fastInsert(KIO::UDSEntry::UDS_NAME, url.fileName());
    uds.fastInsert(KIO::UDSEntry::UDS_URL, url.url());
    uds.fastInsert(KIO::UDSEntry::UDS_LOCAL_PATH, filePath);

    return uds;
}

}
#endif
