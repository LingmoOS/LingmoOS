/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_TUN_DEVICE_P_H
#define NETWORKMANAGERQT_TUN_DEVICE_P_H

#include "device_p.h"
#include "tundevice.h"
#include "tundeviceinterface.h"

namespace NetworkManager
{
class TunDevicePrivate : public DevicePrivate
{
    Q_OBJECT
public:
    TunDevicePrivate(const QString &path, TunDevice *q);
    ~TunDevicePrivate() override;

    OrgFreedesktopNetworkManagerDeviceTunInterface iface;
    qlonglong owner;
    qlonglong group;
    QString mode;
    bool multiQueue;
    bool noPi;
    bool vnetHdr;
    QString hwAddress;

    Q_DECLARE_PUBLIC(TunDevice)
protected:
    /**
     * When subclassing make sure to call the parent class method
     * if the property was not useful to your new class
     */
    void propertyChanged(const QString &property, const QVariant &value) override;
};

}

#endif
