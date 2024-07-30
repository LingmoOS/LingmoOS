/*
    SPDX-FileCopyrightText: 2012-2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_MACVLAN_DEVICE_P_H
#define NETWORKMANAGERQT_MACVLAN_DEVICE_P_H

#include "device_p.h"
#include "macvlandevice.h"
#include "macvlandeviceinterface.h"
#include "manager_p.h"

namespace NetworkManager
{
class MacVlanDevicePrivate : public DevicePrivate
{
    Q_OBJECT
public:
    MacVlanDevicePrivate(const QString &path, MacVlanDevice *q);
    ~MacVlanDevicePrivate() override;

    OrgFreedesktopNetworkManagerDeviceMacvlanInterface iface;
    QString mode;
    bool noPromisc;
    QString parent;

    Q_DECLARE_PUBLIC(MacVlanDevice)
protected:
    /**
     * When subclassing make sure to call the parent class method
     * if the property was not useful to your new class
     */
    void propertyChanged(const QString &property, const QVariant &value) override;
};

}

#endif
