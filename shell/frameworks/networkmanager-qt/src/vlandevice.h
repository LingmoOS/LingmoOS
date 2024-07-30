/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_VLAN_DEVICE_H
#define NETWORKMANAGERQT_VLAN_DEVICE_H

#include "device.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

namespace NetworkManager
{
class VlanDevicePrivate;

/**
 * A vlan device interface
 */
class NETWORKMANAGERQT_EXPORT VlanDevice : public Device
{
    Q_OBJECT
    Q_PROPERTY(bool carrier READ carrier NOTIFY carrierChanged)
    Q_PROPERTY(QString hwAddress READ hwAddress NOTIFY hwAddressChanged)
    Q_PROPERTY(uint vlanId READ vlanId NOTIFY vlanIdChanged)
    Q_PROPERTY(NetworkManager::Device::Ptr parent READ parent NOTIFY parentChanged)

public:
    typedef QSharedPointer<VlanDevice> Ptr;
    typedef QList<Ptr> List;

    explicit VlanDevice(const QString &path, QObject *parent = nullptr);
    ~VlanDevice() override;

    Type type() const override;

    /**
     * Indicates whether the physical carrier is found
     */
    bool carrier() const;
    /**
     * Hardware address of the device
     */
    QString hwAddress() const;
    /**
     * The parent device of this VLAN device
     * @since 5.8.0
     */
    NetworkManager::Device::Ptr parent() const;
    /**
     * The VLAN ID of this VLAN interface
     */
    uint vlanId() const;

Q_SIGNALS:
    /**
     * Emitted when the carrier of this device has changed
     */
    void carrierChanged(bool plugged);
    /**
     * Emitted when the hardware address of this device has changed
     */
    void hwAddressChanged(const QString &address);
    /**
     * Emitted when the parent device of this device has changed
     */
    void parentChanged(const QString &path);
    /**
     * Emitted when the VLAN ID of this device has changed
     */
    void vlanIdChanged(uint id);

private:
    Q_DECLARE_PRIVATE(VlanDevice)
};

}

#endif
