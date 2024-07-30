/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_INFINIBAND_DEVICE_H
#define NETWORKMANAGERQT_INFINIBAND_DEVICE_H

#include "device.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

namespace NetworkManager
{
class InfinibandDevicePrivate;

/**
 * An infiniband device interface
 */
class NETWORKMANAGERQT_EXPORT InfinibandDevice : public Device
{
    Q_OBJECT
    Q_PROPERTY(bool carrier READ carrier NOTIFY carrierChanged)
    Q_PROPERTY(QString hwAddress READ hwAddress NOTIFY hwAddressChanged)

public:
    typedef QSharedPointer<InfinibandDevice> Ptr;
    typedef QList<Ptr> List;
    explicit InfinibandDevice(const QString &path, QObject *parent = nullptr);
    ~InfinibandDevice() override;

    Type type() const override;

    /**
     * Indicates whether the physical carrier is found
     */
    bool carrier() const;
    /**
     * Hardware address of the device
     */
    QString hwAddress() const;

Q_SIGNALS:
    /**
     * Emitted when the carrier of this device has changed
     */
    void carrierChanged(bool plugged);
    /**
     * Emitted when the hardware address of this device has changed
     */
    void hwAddressChanged(const QString &address);

private:
    Q_DECLARE_PRIVATE(InfinibandDevice)
};

}

#endif
