/*
    SPDX-FileCopyrightText: 2012-2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_GRE_DEVICE_P_H
#define NETWORKMANAGERQT_GRE_DEVICE_P_H

#include "device_p.h"
#include "gredevice.h"
#include "manager.h"
#include "manager_p.h"

#include "gredeviceinterface.h"

namespace NetworkManager
{
class GreDevicePrivate : public DevicePrivate
{
    Q_OBJECT
public:
    GreDevicePrivate(const QString &path, GreDevice *q);
    ~GreDevicePrivate() override;

    OrgFreedesktopNetworkManagerDeviceGreInterface iface;
    ushort inputFlags;
    ushort outputFlags;
    uint inputKey;
    uint outputKey;
    QString localEnd;
    QString remoteEnd;
    QString parent;
    bool pathMtuDiscovery;
    uchar tos;
    uchar ttl;

    Q_DECLARE_PUBLIC(GreDevice)
protected:
    /**
     * When subclassing make sure to call the parent class method
     * if the property was not useful to your new class
     */
    void propertyChanged(const QString &property, const QVariant &value) override;
};

}

#endif
