/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_OLPCMESHDEVICE_P_H
#define NETWORKMANAGERQT_OLPCMESHDEVICE_P_H

#include "dbus/olpcmeshdeviceinterface.h"
#include "device_p.h"

namespace NetworkManager
{
class OlpcMeshDevicePrivate : public DevicePrivate
{
    Q_OBJECT
public:
    explicit OlpcMeshDevicePrivate(const QString &path, OlpcMeshDevice *q);
    OrgFreedesktopNetworkManagerDeviceOlpcMeshInterface iface;
    uint activeChannel;
    QString companion;
    QString hardwareAddress;

    Q_DECLARE_PUBLIC(OlpcMeshDevice)
protected:
    /**
     * When subclassing make sure to call the parent class method
     * if the property was not useful to your new class
     */
    void propertyChanged(const QString &property, const QVariant &value) override;
};

}

#endif
