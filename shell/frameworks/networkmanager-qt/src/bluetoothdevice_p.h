/*
    SPDX-FileCopyrightText: 2011 Lamarque Souza <lamarque@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_BLUETOOTH_DEVICE_P_H
#define NETWORKMANAGERQT_BLUETOOTH_DEVICE_P_H

#include "dbus/bluetoothdeviceinterface.h"
#include "modemdevice_p.h"

namespace NetworkManager
{
class BluetoothDevicePrivate : public ModemDevicePrivate
{
    Q_OBJECT
public:
    explicit BluetoothDevicePrivate(const QString &path, BluetoothDevice *q);
    OrgFreedesktopNetworkManagerDeviceBluetoothInterface btIface;
    BluetoothDevice::Capabilities btCapabilities;
    QString hardwareAddress;
    QString name;

    Q_DECLARE_PUBLIC(BluetoothDevice)
protected:
    /**
     * When subclassing make sure to call the parent class method
     * if the property was not useful to your new class
     */
    void propertyChanged(const QString &property, const QVariant &value) override;
};

}
#endif // NETWORKMANAGERQT__BLUETOOTH_DEVICE_P_H
