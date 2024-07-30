/*
    SPDX-FileCopyrightText: 2011 Lamarque V. Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_DHCP4CONFIG_P_H
#define NETWORKMANAGERQT_DHCP4CONFIG_P_H

#include "dbus/dhcp4configinterface.h"
#include "dhcp4config.h"

namespace NetworkManager
{
class Dhcp4ConfigPrivate : public QObject
{
    Q_OBJECT
public:
    Dhcp4ConfigPrivate(const QString &path, Dhcp4Config *q);
    ~Dhcp4ConfigPrivate() override;
    OrgFreedesktopNetworkManagerDHCP4ConfigInterface dhcp4Iface;
    QString myPath;
    QVariantMap options;

    Q_DECLARE_PUBLIC(Dhcp4Config)
    Dhcp4Config *q_ptr;
protected Q_SLOTS:
    void dbusPropertiesChanged(const QString &interfaceName, const QVariantMap &properties, const QStringList &invalidatedProperties);
    void dhcp4PropertiesChanged(const QVariantMap &);
};

} // namespace NetworkManager

#endif
