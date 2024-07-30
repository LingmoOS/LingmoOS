/*
    SPDX-FileCopyrightText: 2012-2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_INFINIBAND_DEVICE_P_H
#define NETWORKMANAGERQT_INFINIBAND_DEVICE_P_H

#include "device_p.h"
#include "infinibanddevice.h"
#include "infinibanddeviceinterface.h"
#include "manager_p.h"

namespace NetworkManager
{
class InfinibandDevicePrivate : public DevicePrivate
{
    Q_OBJECT
public:
    InfinibandDevicePrivate(const QString &path, InfinibandDevice *q);
    ~InfinibandDevicePrivate() override;

    OrgFreedesktopNetworkManagerDeviceInfinibandInterface iface;
    bool carrier;
    QString hwAddress;

    Q_DECLARE_PUBLIC(InfinibandDevice)
protected:
    /**
     * When subclassing make sure to call the parent class method
     * if the property was not useful to your new class
     */
    void propertyChanged(const QString &property, const QVariant &value) override;
};

}

#endif
