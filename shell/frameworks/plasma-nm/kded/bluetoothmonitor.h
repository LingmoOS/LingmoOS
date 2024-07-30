/*
    SPDX-FileCopyrightText: 2011 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2014 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_BLUETOOTH_MONITOR_H
#define PLASMA_NM_BLUETOOTH_MONITOR_H
#include "mobileconnectionwizard.h"
#include <ModemManagerQt/Manager>

#include <QObject>

class BluetoothMonitor : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothMonitor(QObject *parent);
    ~BluetoothMonitor() override;

    bool bluetoothConnectionExists(const QString &bdAddr, const QString &service);
    void addBluetoothConnection(const QString &bdAddr, const QString &service, const QString &connectionName);
};
#endif
