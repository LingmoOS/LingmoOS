/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef NETWORKMANAGERQT_MODEMDEVICE_P_H
#define NETWORKMANAGERQT_MODEMDEVICE_P_H

#include "dbus/modemdeviceinterface.h"
#include "device_p.h"

namespace NetworkManager
{
class ModemDevicePrivate : public DevicePrivate
{
    Q_OBJECT
public:
    explicit ModemDevicePrivate(const QString &path, ModemDevice *q);

    void initModemProperties();

    OrgFreedesktopNetworkManagerDeviceModemInterface modemIface;
    NetworkManager::ModemDevice::Capabilities modemCapabilities;
    NetworkManager::ModemDevice::Capabilities currentCapabilities;

    Q_DECLARE_PUBLIC(ModemDevice)
protected:
    /**
     * When subclassing make sure to call the parent class method
     * if the property was not useful to your new class
     */
    void propertyChanged(const QString &property, const QVariant &value) override;
};

} // namespace NetworkManager

#endif
