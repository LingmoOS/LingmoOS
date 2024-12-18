// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef OCEANMOTYPES_P_H
#define OCEANMOTYPES_P_H

#include "oceanmotypes.h"
#include "dtkdemo_global.h"
#include <QDBusArgument>
#include <QList>

OCEANMO_BEGIN_NAMESPACE

struct UserPath_p
{
    uint user_id;
    QDBusObjectPath path;
    static void registerMetaType();
    bool operator==(const UserPath_p &up) const { return up.user_id == user_id && up.path == path; }
    operator UserPath();
    operator UserPath() const;
};

using UserPathList_p = QList<UserPath_p>;

QDBusArgument &operator<<(QDBusArgument &arg, const UserPath_p &path);
const QDBusArgument &operator>>(const QDBusArgument &arg, UserPath_p &path);

OCEANMO_END_NAMESPACE

Q_DECLARE_METATYPE(DTK_DEMO_NAMESPACE::UserPath_p)

#endif
