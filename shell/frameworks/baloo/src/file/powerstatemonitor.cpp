/*
    This file is part of the KDE Project
    SPDX-FileCopyrightText: 2008 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2010-15 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "powerstatemonitor.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

using namespace Baloo;

PowerStateMonitor::PowerStateMonitor(QObject* parent)
    : QObject(parent)
    , m_isOnBattery(true)
{
    // monitor the powermanagement to not drain the battery
    QDBusConnection::sessionBus().connect(QStringLiteral("org.freedesktop.PowerManagement"),
                                          QStringLiteral("/org/freedesktop/PowerManagement"),
                                          QStringLiteral("org.freedesktop.PowerManagement"),
                                          QStringLiteral("PowerSaveStatusChanged"),
                                          this, SLOT(slotPowerManagementStatusChanged(bool)));

    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.PowerManagement"),
                                                      QStringLiteral("/org/freedesktop/PowerManagement"),
                                                      QStringLiteral("org.freedesktop.PowerManagement"),
                                                      QStringLiteral("GetPowerSaveStatus"));

    QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(msg);
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, [&](QDBusPendingCallWatcher* watch) {
        QDBusPendingReply<bool> reply = *watch;
        if (!reply.isError()) {
            bool onBattery = reply.argumentAt<0>();
            slotPowerManagementStatusChanged(onBattery);
        } else {
            slotPowerManagementStatusChanged(false);
        }
        watch->deleteLater();
    });
}

void PowerStateMonitor::slotPowerManagementStatusChanged(bool conserveResources)
{
    if (m_isOnBattery != conserveResources) {
        m_isOnBattery = conserveResources;
        Q_EMIT powerManagementStatusChanged(conserveResources);
    }
}

#include "moc_powerstatemonitor.cpp"
