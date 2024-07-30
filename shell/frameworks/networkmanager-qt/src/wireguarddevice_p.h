/*
    SPDX-FileCopyrightText: 2019 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_WIREGUARD_DEVICE_P_H
#define NETWORKMANAGERQT_WIREGUARD_DEVICE_P_H

#include "device_p.h"
#include "manager.h"
#include "wireguarddevice.h"

#include "wireguarddeviceinterface.h"

namespace NetworkManager
{
class WireGuardDevicePrivate : public DevicePrivate
{
public:
    WireGuardDevicePrivate(const QString &path, WireGuardDevice *q);
    ~WireGuardDevicePrivate() override;

    OrgFreedesktopNetworkManagerDeviceWireGuardInterface iface;
    QByteArray publicKey;
    uint listenPort;
    uint fwMark;

    Q_DECLARE_PUBLIC(WireGuardDevice)
protected:
    /**
     * When subclassing make sure to call the parent class method
     * if the property was not useful to your new class
     */
    void propertyChanged(const QString &property, const QVariant &value) override;
};
}

#endif
