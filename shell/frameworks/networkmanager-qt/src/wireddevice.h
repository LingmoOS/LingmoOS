/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2011-2013 Lamarque V. Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_WIREDDEVICE_H
#define NETWORKMANAGERQT_WIREDDEVICE_H

#include <networkmanagerqt/networkmanagerqt_export.h>

#include "device.h"

namespace NetworkManager
{
class WiredDevicePrivate;

/**
 * A wired device interface
 */
class NETWORKMANAGERQT_EXPORT WiredDevice : public Device
{
    Q_OBJECT
    Q_PROPERTY(QString hardwareAddress READ hardwareAddress)
    Q_PROPERTY(QString permanentHardwareAddress READ permanentHardwareAddress)
    Q_PROPERTY(bool carrier READ carrier NOTIFY carrierChanged)
    Q_PROPERTY(int bitRate READ bitRate NOTIFY bitRateChanged)
    Q_PROPERTY(QStringList s390SubChannels READ s390SubChannels NOTIFY s390SubChannelsChanged)

public:
    typedef QSharedPointer<WiredDevice> Ptr;
    typedef QList<Ptr> List;
    explicit WiredDevice(const QString &path, QObject *parent = nullptr);
    ~WiredDevice() override;
    /**
     * Return the type
     */
    Type type() const override;
    /**
     * Active hardware address of the device
     */
    QString hardwareAddress() const;
    /**
     * Permanent hardware address of the device
     */
    QString permanentHardwareAddress() const;
    /**
     * Design speed of the device, in megabits/second (Mb/s)
     */
    int bitRate() const;
    /**
     * Indicates whether the physical carrier is found (e.g. whether a cable is plugged in or not)
     */
    bool carrier() const;
    /**
     * Array of S/390 subchannels for S/390 or z/Architecture devices
     */
    QStringList s390SubChannels() const;

Q_SIGNALS:
    /**
     * Emitted when the design speed of the device has changed
     */
    void bitRateChanged(int bitRate);
    /**
     * Emitted when the carrier of this device has changed
     */
    void carrierChanged(bool plugged);
    /**
     * Emitted when the hardware address of this device has changed
     */
    void hardwareAddressChanged(const QString &hwAddress);
    /**
     * Emitted when the permanent hardware address of this device has changed
     */
    void permanentHardwareAddressChanged(const QString &permHwAddress);
    /*
     * Emitted when the array of s390SubChannels has changed
     */
    void s390SubChannelsChanged(const QStringList &channels);

private:
    Q_DECLARE_PRIVATE(WiredDevice)
};

}

#endif // NETWORKMANAGERQT_WIREDDEVICE_H
