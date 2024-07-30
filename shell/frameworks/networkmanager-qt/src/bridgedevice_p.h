/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_BRIDGE_DEVICE_P_H
#define NETWORKMANAGERQT_BRIDGE_DEVICE_P_H

#include "bridgedevice.h"
#include "device_p.h"
#include "manager.h"

#include "bridgedeviceinterface.h"

namespace NetworkManager
{
class BridgeDevicePrivate : public DevicePrivate
{
public:
    BridgeDevicePrivate(const QString &path, BridgeDevice *q);
    ~BridgeDevicePrivate() override;

    OrgFreedesktopNetworkManagerDeviceBridgeInterface iface;
    bool carrier;
    QString hwAddress;
    QStringList slaves;

    Q_DECLARE_PUBLIC(BridgeDevice)
protected:
    /**
     * When subclassing make sure to call the parent class method
     * if the property was not useful to your new class
     */
    void propertyChanged(const QString &property, const QVariant &value) override;
};
}

#endif
