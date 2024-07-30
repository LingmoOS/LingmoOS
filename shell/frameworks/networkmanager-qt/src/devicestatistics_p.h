/*
    SPDX-FileCopyrightText: 2017 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_DEVICE_STATISTICS_P_H
#define NETWORKMANAGERQT_DEVICE_STATISTICS_P_H

#include "dbus/devicestatisticsinterface.h"
#include "devicestatistics.h"

namespace NetworkManager
{
class DeviceStatisticsPrivate : public QObject
{
    Q_OBJECT
public:
    DeviceStatisticsPrivate(const QString &path, DeviceStatistics *q);

    OrgFreedesktopNetworkManagerDeviceStatisticsInterface iface;
    QString uni;
    uint refreshRateMs;
    qulonglong rxBytes;
    qulonglong txBytes;

    Q_DECLARE_PUBLIC(DeviceStatistics)
    DeviceStatistics *q_ptr;
private Q_SLOTS:
    void dbusPropertiesChanged(const QString &interfaceName, const QVariantMap &properties, const QStringList &invalidatedProperties);
    void propertiesChanged(const QVariantMap &properties);
};

} // namespace NetworkManager

#endif // NETWORKMANAGERQT_DEVICE_STATISTICS_P_H
