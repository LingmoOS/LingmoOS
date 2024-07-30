/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_AVAILABLE_DEVICES_H
#define PLASMA_NM_AVAILABLE_DEVICES_H

#include <QObject>

#include <qqmlregistration.h>

#include <NetworkManagerQt/Device>

class AvailableDevices : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    /**
     * Return true when there is present wired device
     */
    Q_PROPERTY(bool wiredDeviceAvailable READ isWiredDeviceAvailable NOTIFY wiredDeviceAvailableChanged)
    /**
     * Return true when there is present wireless device
     */
    Q_PROPERTY(bool wirelessDeviceAvailable READ isWirelessDeviceAvailable NOTIFY wirelessDeviceAvailableChanged)

    /**
     * Return true when there is present modem device
     */
    Q_PROPERTY(bool modemDeviceAvailable READ isModemDeviceAvailable NOTIFY modemDeviceAvailableChanged)
    /**
     * Return true when there is present bluetooth device
     * Bluetooth device is visible for NetworkManager only when there is some Bluetooth connection
     */
    Q_PROPERTY(bool bluetoothDeviceAvailable READ isBluetoothDeviceAvailable NOTIFY bluetoothDeviceAvailableChanged)
public:
    explicit AvailableDevices(QObject *parent = nullptr);
    ~AvailableDevices() override;

public Q_SLOTS:
    bool isWiredDeviceAvailable() const;
    bool isWirelessDeviceAvailable() const;
    bool isModemDeviceAvailable() const;
    bool isBluetoothDeviceAvailable() const;

private Q_SLOTS:
    void deviceAdded(const QString &dev);
    void deviceRemoved();

Q_SIGNALS:
    void wiredDeviceAvailableChanged(bool available);
    void wirelessDeviceAvailableChanged(bool available);
    void modemDeviceAvailableChanged(bool available);
    void bluetoothDeviceAvailableChanged(bool available);

private:
    bool m_wiredDeviceAvailable = false;
    bool m_wirelessDeviceAvailable = false;
    bool m_modemDeviceAvailable = false;
    bool m_bluetoothDeviceAvailable = false;
};

#endif // PLASMA_NM_AVAILABLE_DEVICES_H
