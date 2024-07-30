/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_WIRELESSDEVICE_P_H
#define NETWORKMANAGERQT_WIRELESSDEVICE_P_H

#include "dbus/wirelessdeviceinterface.h"
#include "device_p.h"

namespace NetworkManager
{
class WirelessDevicePrivate : public DevicePrivate
{
    Q_OBJECT
public:
    explicit WirelessDevicePrivate(const QString &path, WirelessDevice *q);
    OrgFreedesktopNetworkManagerDeviceWirelessInterface wirelessIface;
    QString permanentHardwareAddress;
    QString hardwareAddress;
    QHash<QString, WirelessNetwork::Ptr> networks;
    QMap<QString, AccessPoint::Ptr> apMap;
    // index of the active AP or -1 if none
    AccessPoint::Ptr activeAccessPoint;
    WirelessDevice::OperationMode mode;
    uint bitRate;
    WirelessDevice::Capabilities wirelessCapabilities;
    QDateTime lastScan;
    QDateTime lastRequestScan;

    Q_DECLARE_PUBLIC(WirelessDevice)
protected:
    /**
     * When subclassing make sure to call the parent class method
     * if the property was not useful to your new class
     */
    void propertyChanged(const QString &property, const QVariant &value) override;

protected Q_SLOTS:
    void accessPointAdded(const QDBusObjectPath &);
    void accessPointRemoved(const QDBusObjectPath &);
    void removeNetwork(const QString &network);
};

}

#endif
