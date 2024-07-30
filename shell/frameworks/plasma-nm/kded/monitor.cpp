/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "monitor.h"

#include <QDBusConnection>

Monitor::Monitor(QObject *parent)
    : QObject(parent)
    , m_bluetoothMonitor(new BluetoothMonitor(this))
{
    m_modemMonitor = new ModemMonitor(this);

    QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.plasmanetworkmanagement"));
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/org/kde/plasmanetworkmanagement"), this, QDBusConnection::ExportScriptableContents);
}

Monitor::~Monitor()
{
    delete m_bluetoothMonitor;
    delete m_modemMonitor;
}

bool Monitor::bluetoothConnectionExists(const QString &bdAddr, const QString &service)
{
    return m_bluetoothMonitor->bluetoothConnectionExists(bdAddr, service);
}

void Monitor::addBluetoothConnection(const QString &bdAddr, const QString &service, const QString &connectionName)
{
    m_bluetoothMonitor->addBluetoothConnection(bdAddr, service, connectionName);
}

void Monitor::unlockModem(const QString &modem)
{
    qDebug() << "unlocking " << modem;
    m_modemMonitor->unlockModem(modem);
}

#include "moc_monitor.cpp"
