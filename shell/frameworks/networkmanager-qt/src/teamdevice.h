/*
    SPDX-FileCopyrightText: 2013 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_TEAM_DEVICE_H
#define NETWORKMANAGERQT_TEAM_DEVICE_H

#include "device.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

namespace NetworkManager
{
class TeamDevicePrivate;

/**
 * A team device interface
 */
class NETWORKMANAGERQT_EXPORT TeamDevice : public Device
{
    Q_OBJECT
    Q_PROPERTY(bool carrier READ carrier NOTIFY carrierChanged)
    Q_PROPERTY(QString hwAddress READ hwAddress NOTIFY hwAddressChanged)
    Q_PROPERTY(QStringList slaves READ slaves NOTIFY slavesChanged)
    Q_PROPERTY(QString config READ config NOTIFY configChanged)

public:
    typedef QSharedPointer<TeamDevice> Ptr;
    typedef QList<Ptr> List;

    explicit TeamDevice(const QString &path, QObject *parent = nullptr);
    ~TeamDevice() override;

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
     * Devices which are currently slaved to this device
     */
    QStringList slaves() const;

    /**
     * The JSON configuration currently applied on the device.
     */
    QString config() const;

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
     * Emitted when the list of devices slaved to this device has changed
     */
    void slavesChanged(const QStringList &slaves);

    /**
     * Emitted when the JSON confugration which is currently applied has changed
     */
    void configChanged(const QString &config);

private:
    Q_DECLARE_PRIVATE(TeamDevice)
};

}

#endif
