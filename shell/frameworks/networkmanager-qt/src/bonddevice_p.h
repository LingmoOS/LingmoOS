/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_BOND_DEVICE_P_H
#define NETWORKMANAGERQT_BOND_DEVICE_P_H

#include "bonddevice.h"
#include "dbus/bonddeviceinterface.h"
#include "device_p.h"

namespace NetworkManager
{
class BondDevicePrivate : public DevicePrivate
{
    Q_OBJECT
public:
    BondDevicePrivate(const QString &path, BondDevice *q);
    ~BondDevicePrivate() override;

    OrgFreedesktopNetworkManagerDeviceBondInterface iface;
    bool carrier;
    QString hwAddress;
    QStringList slaves;

    Q_DECLARE_PUBLIC(BondDevice)
protected:
    /**
     * When subclassing make sure to call the parent class method
     * if the property was not useful to your new class
     */
    void propertyChanged(const QString &property, const QVariant &value) override;
};
}
#endif // NETWORKMANAGERQT__BLUETOOTH_DEVICE_P_H
