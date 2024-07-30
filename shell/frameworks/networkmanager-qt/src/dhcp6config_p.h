/*
    SPDX-FileCopyrightText: 2011 Lamarque V. Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_DHCP6CONFIG_P_H
#define NETWORKMANAGERQT_DHCP6CONFIG_P_H

#include "dbus/dhcp6configinterface.h"
#include "dhcp6config.h"

namespace NetworkManager
{
class Dhcp6ConfigPrivate : public QObject
{
    Q_OBJECT
public:
    Dhcp6ConfigPrivate(const QString &path, Dhcp6Config *q);
    ~Dhcp6ConfigPrivate() override;
    OrgFreedesktopNetworkManagerDHCP6ConfigInterface dhcp6Iface;
    QString path;
    QVariantMap options;

    Q_DECLARE_PUBLIC(Dhcp6Config)
    Dhcp6Config *q_ptr;
protected Q_SLOTS:
    void dbusPropertiesChanged(const QString &interfaceName, const QVariantMap &properties, const QStringList &invalidatedProperties);
    void dhcp6PropertiesChanged(const QVariantMap &);
};

} // namespace NetworkManager

#endif
