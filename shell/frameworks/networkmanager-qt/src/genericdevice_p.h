/*
    SPDX-FileCopyrightText: 2012-2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_GENERIC_DEVICE_P_H
#define NETWORKMANAGERQT_GENERIC_DEVICE_P_H

#include "device_p.h"
#include "genericdevice.h"
#include "genericdeviceinterface.h"
#include "manager_p.h"

namespace NetworkManager
{
class GenericDevicePrivate : public DevicePrivate
{
    Q_OBJECT
public:
    GenericDevicePrivate(const QString &path, GenericDevice *q);
    ~GenericDevicePrivate() override;

    OrgFreedesktopNetworkManagerDeviceGenericInterface iface;
    QString hwAddress;
    QString typeDescription;

    Q_DECLARE_PUBLIC(GenericDevice)
protected:
    /**
     * When subclassing make sure to call the parent class method
     * if the property was not useful to your new class
     */
    void propertyChanged(const QString &property, const QVariant &value) override;
};

}

#endif
