/*
    SPDX-FileCopyrightText: 2012-2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_VLAN_DEVICE_P_H
#define NETWORKMANAGERQT_VLAN_DEVICE_P_H

#include "device_p.h"
#include "manager_p.h"
#include "vlandevice.h"
#include "vlandeviceinterface.h"

namespace NetworkManager
{
class VlanDevicePrivate : public DevicePrivate
{
    Q_OBJECT
public:
    VlanDevicePrivate(const QString &path, VlanDevice *q);
    ~VlanDevicePrivate() override;

    OrgFreedesktopNetworkManagerDeviceVlanInterface iface;
    bool carrier;
    QString hwAddress;
    QString parent;
    uint vlanId;

    Q_DECLARE_PUBLIC(VlanDevice)
protected:
    /**
     * When subclassing make sure to call the parent class method
     * if the property was not useful to your new class
     */
    void propertyChanged(const QString &property, const QVariant &value) override;
};

}

#endif
