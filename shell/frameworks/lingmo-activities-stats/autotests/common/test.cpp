/*
    SPDX-FileCopyrightText: 2013, 2014, 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include "common/dbus/common.h"

Test::Test(QObject *parent)
    : QObject(parent)
{
}

bool Test::inEmptySession()
{
    const QStringList services = QDBusConnection::sessionBus().interface()->registeredServiceNames();

    for (const QString &service : services) {
        bool kdeServiceAndNotKAMD = service.startsWith(QStringLiteral("org.kde")) && service != KAMD_DBUS_SERVICE;

        if (kdeServiceAndNotKAMD) {
            return false;
        }
    }

    return true;
}

bool Test::isActivityManagerRunning()
{
    return QDBusConnection::sessionBus().interface()->isServiceRegistered(KAMD_DBUS_SERVICE);
}

#include "moc_test.cpp"
