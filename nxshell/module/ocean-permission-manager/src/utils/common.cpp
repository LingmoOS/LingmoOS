// Copyright (C) 2023 ~ 2023 Lingmo Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDebug>
#include <QVariant>

#include <polkit-qt5-1/PolkitQt1/Authority>

#include "common.h"

using namespace PolkitQt1;

bool checkAuth(const QString &actionId, const int &pid)
{
    Authority::Result authenticationResult;
    authenticationResult = Authority::instance()->checkAuthorizationSync(actionId, UnixProcessSubject(pid),
                                                           Authority::AllowUserInteraction);

    qWarning() << "authenticationResult: " << authenticationResult;
    if (Authority::Result::Yes != authenticationResult) {
        return false;
    }

    return true;
}

UserType checkUserIsAdmin()
{
    struct passwd *user = getpwuid(getuid());
    if (!user) {
        qWarning() << "call getpwuid failed";
        return UserTypeInvalid;
    }

    QDBusInterface inter = QDBusInterface("org.lingmo.ocean.Accounts1",
                "/org/lingmo/ocean/Accounts1",
                "org.lingmo.ocean.Accounts1",
                QDBusConnection::systemBus());

    QDBusReply<QString> reply = inter.call("FindUserByName", QString(user->pw_name));
    if (!reply.isValid()) {
        qWarning() << "call FindUserByName failed: " << reply.error();
        return UserTypeInvalid;
    }

    QString path = reply.value();
    QDBusInterface inter1 = QDBusInterface("org.lingmo.ocean.Accounts1",
                path,
                "org.lingmo.ocean.Accounts1.User",
                QDBusConnection::systemBus());

    QVariant ret = inter1.property("AccountType");
    return UserType(ret.toInt());
}
