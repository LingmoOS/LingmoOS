/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_VETH_DEVICE_P_H
#define NETWORKMANAGERQT_VETH_DEVICE_P_H

#include "device_p.h"
#include "vethdevice.h"

#include "vethdeviceinterface.h"

namespace NetworkManager
{
class VethDevicePrivate : public DevicePrivate
{
    Q_OBJECT
public:
    VethDevicePrivate(const QString &path, VethDevice *q);
    ~VethDevicePrivate() override;

    OrgFreedesktopNetworkManagerDeviceVethInterface iface;
    QString peer;

    Q_DECLARE_PUBLIC(VethDevice)
protected:
    /**
     * When subclassing make sure to call the parent class method
     * if the property was not useful to your new class
     */
    void propertyChanged(const QString &property, const QVariant &value) override;
};

}

#endif
