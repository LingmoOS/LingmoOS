/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_MONITOR_H
#define PLASMA_NM_MONITOR_H

#include <QDBusPendingCallWatcher>
#include <QObject>

#include "bluetoothmonitor.h"
#include "modemmonitor.h"

class Q_DECL_EXPORT Monitor : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.plasmanetworkmanagement")
public:
    explicit Monitor(QObject *parent);
    ~Monitor() override;

public Q_SLOTS:
    Q_SCRIPTABLE bool bluetoothConnectionExists(const QString &bdAddr, const QString &service);
    Q_SCRIPTABLE void addBluetoothConnection(const QString &bdAddr, const QString &service, const QString &connectionName);
    Q_SCRIPTABLE void unlockModem(const QString &modem);

private:
    BluetoothMonitor *const m_bluetoothMonitor;
    ModemMonitor *m_modemMonitor = nullptr;
};

#endif // PLASMA_NM_MONITOR_H
