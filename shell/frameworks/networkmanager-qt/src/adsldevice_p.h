/*
    SPDX-FileCopyrightText: 2012-2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_ADSL_DEVICE_P_H
#define NETWORKMANAGERQT_ADSL_DEVICE_P_H

#include "adsldevice.h"
#include "device_p.h"
#include "manager.h"
#include "manager_p.h"

#include "adsldeviceinterface.h"

namespace NetworkManager
{
class AdslDevicePrivate : public DevicePrivate
{
    Q_OBJECT
public:
    AdslDevicePrivate(const QString &path, AdslDevice *q);
    ~AdslDevicePrivate() override;

    OrgFreedesktopNetworkManagerDeviceAdslInterface iface;
    bool carrier;

    Q_DECLARE_PUBLIC(AdslDevice)
protected:
    /**
     * When subclassing make sure to call the parent class method
     * if the property was not useful to your new class
     */
    void propertyChanged(const QString &property, const QVariant &value) override;
};
}

#endif
