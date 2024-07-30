/*
    SPDX-FileCopyrightText: 1999 Carsten Pfeiffer <pfeiffer@kde.org>
    SPDX-FileCopyrightText: 1999-2001 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2005 Till Adam <adam@kde.org>
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson@kde.org>
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
    SPDX-FileCopyrightText: 2020 Stefan Brüns <bruns@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BALOO_KIO_COMMON_USERGROUPCACHE_H_
#define BALOO_KIO_COMMON_USERGROUPCACHE_H_

#include <KUser>

#include <QHash>
#include <QString>

namespace Baloo {

class UserGroupCache
{
public:
    QString getUserName(const KUserId &uid) const;
    QString getGroupName(const KGroupId &gid) const;

private:
    mutable QHash<KUserId, QString> mUsercache;
    mutable QHash<KGroupId, QString> mGroupcache;
};

inline QString
UserGroupCache::getUserName(const KUserId &uid) const
{
    if (Q_UNLIKELY(!uid.isValid())) {
        return QString();
    }
    if (!mUsercache.contains(uid)) {
        KUser user(uid);
        QString name = user.loginName();
        if (name.isEmpty()) {
            name = uid.toString();
        }
        mUsercache.insert(uid, name);
        return name;
    }
    return mUsercache[uid];
}

inline QString
UserGroupCache::getGroupName(const KGroupId &gid) const
{
    if (Q_UNLIKELY(!gid.isValid())) {
        return QString();
    }
    if (!mGroupcache.contains(gid)) {
        KUserGroup group(gid);
        QString name = group.name();
        if (name.isEmpty()) {
            name = gid.toString();
        }
        mGroupcache.insert(gid, name);
        return name;
    }
    return mGroupcache[gid];
}

}
#endif
