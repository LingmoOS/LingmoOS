/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_WIREDDEVICE_P_H
#define NETWORKMANAGERQT_WIREDDEVICE_P_H

#include "dbus/wireddeviceinterface.h"
#include "device_p.h"

namespace NetworkManager
{
class WiredDevicePrivate : public DevicePrivate
{
    Q_OBJECT
public:
    explicit WiredDevicePrivate(const QString &path, WiredDevice *q);
    ~WiredDevicePrivate() override;
    OrgFreedesktopNetworkManagerDeviceWiredInterface wiredIface;
    QString hardwareAddress;
    QString permanentHardwareAddress;
    QStringList s390SubChannels;
    int bitrate;
    bool carrier;

    Q_DECLARE_PUBLIC(WiredDevice)
protected:
    /**
     * When subclassing make sure to call the parent class method
     * if the property was not useful to your new class
     */
    void propertyChanged(const QString &property, const QVariant &value) override;
};

} // namespace NetworkManager

#endif
