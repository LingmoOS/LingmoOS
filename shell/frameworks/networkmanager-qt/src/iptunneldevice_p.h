/*
    SPDX-FileCopyrightText: 2017 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_IPTUNNEL_DEVICE_P_H
#define NETWORKMANAGERQT_IPTUNNEL_DEVICE_P_H

#include "device_p.h"
#include "iptunneldevice.h"
#include "manager.h"
#include "manager_p.h"

#include "iptunneldeviceinterface.h"

namespace NetworkManager
{
class IpTunnelDevicePrivate : public DevicePrivate
{
    Q_OBJECT
public:
    IpTunnelDevicePrivate(const QString &path, IpTunnelDevice *q);
    ~IpTunnelDevicePrivate() override;

    OrgFreedesktopNetworkManagerDeviceIPTunnelInterface iface;
    uchar encapsulationLimit;
    uint flowLabel;
    QString inputKey;
    QString local;
    uint mode;
    QString outputKey;
    QString parent;
    bool pathMtuDiscovery;
    QString remote;
    uchar tos;
    uchar ttl;

    Q_DECLARE_PUBLIC(IpTunnelDevice)
protected:
    /**
     * When subclassing make sure to call the parent class method
     * if the property was not useful to your new class
     */
    void propertyChanged(const QString &property, const QVariant &value) override;
};

}

#endif
