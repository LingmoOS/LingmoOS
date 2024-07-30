/*
    SPDX-FileCopyrightText: 2009 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_MODEM_MONITOR_H
#define PLASMA_NM_MODEM_MONITOR_H

#include <QDBusPendingCallWatcher>
#include <QObject>

#include <ModemManagerQt/Modem>
#include <ModemManagerQt/ModemDevice>

class ModemMonitorPrivate;

/**
 * Monitors modem hardware and provides a PIN unlock dialog
 */
class Q_DECL_EXPORT ModemMonitor : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ModemMonitor)
public:
    explicit ModemMonitor(QObject *parent);
    ~ModemMonitor() override;

public Q_SLOTS:
    void unlockModem(const QString &modemUni);
private Q_SLOTS:
    void requestPin(MMModemLock lock);
    void onSendPinArrived(QDBusPendingCallWatcher *);

private:
    ModemMonitorPrivate *const d_ptr;
};

#endif // PLASMA_NM_MODEM_MONITOR_H
