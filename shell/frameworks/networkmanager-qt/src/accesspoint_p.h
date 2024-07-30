/*
    SPDX-FileCopyrightText: 2008 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2013-2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_ACCESSPOINT_P_H
#define NETWORKMANAGERQT_ACCESSPOINT_P_H

#include "accesspoint.h"
#include "dbus/accesspointinterface.h"

namespace NetworkManager
{
class AccessPointPrivate : public QObject
{
    Q_OBJECT
public:
    AccessPointPrivate(const QString &path, AccessPoint *q);

    OrgFreedesktopNetworkManagerAccessPointInterface iface;
    QString uni;
    AccessPoint::Capabilities capabilities;
    AccessPoint::WpaFlags wpaFlags;
    AccessPoint::WpaFlags rsnFlags;
    QString ssid;
    QByteArray rawSsid;
    uint frequency;
    QString hardwareAddress;
    uint maxBitRate;
    AccessPoint::OperationMode mode;
    int signalStrength;

    int lastSeen;

    NetworkManager::AccessPoint::Capabilities convertCapabilities(int caps);
    NetworkManager::AccessPoint::WpaFlags convertWpaFlags(uint theirFlags);

    Q_DECLARE_PUBLIC(AccessPoint)
    AccessPoint *q_ptr;
private Q_SLOTS:
    void dbusPropertiesChanged(const QString &interfaceName, const QVariantMap &properties, const QStringList &invalidatedProperties);
    void propertiesChanged(const QVariantMap &properties);
};

} // namespace NetworkManager

#endif // NETWORKMANAGERQT_ACCESSPOINT_P_H
