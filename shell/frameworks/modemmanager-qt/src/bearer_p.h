/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_BEARER_P_H
#define MODEMMANAGERQT_BEARER_P_H

#include "bearer.h"
#include "dbus/bearerinterface.h"

namespace ModemManager
{
class BearerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit BearerPrivate(const QString &path, Bearer *q);

    OrgFreedesktopModemManager1BearerInterface bearerIface;
    QString uni;
    QString bearerInterface;
    bool isConnected;
    bool isSuspended;
    mutable ModemManager::IpConfig ipv4Config;
    mutable ModemManager::IpConfig ipv6Config;
    uint ipTimeout;
    QVariantMap bearerProperties;

    ModemManager::IpConfig ipConfigFromMap(const QVariantMap &map);

    Q_DECLARE_PUBLIC(Bearer)
    Bearer *q_ptr;
private Q_SLOTS:
    void onPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProps);
};

} // namespace ModemManager

#endif // MODEMMANAGERQT_BEARER_P_H
