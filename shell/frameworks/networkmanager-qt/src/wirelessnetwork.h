/*
    SPDX-FileCopyrightText: 2009, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2011-2013 Lamarque V. Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_WIRELESSNETWORK_H
#define NETWORKMANAGERQT_WIRELESSNETWORK_H

#include "accesspoint.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QObject>
#include <QSharedPointer>

namespace NetworkManager
{
class WirelessDevice;
class WirelessNetworkPrivate;

/**
 * This class represents a wireless network, which aggregates all
 * access points with the same SSID
 */
class NETWORKMANAGERQT_EXPORT WirelessNetwork : public QObject
{
    Q_OBJECT
    friend class WirelessDevice;
    friend class WirelessDevicePrivate;

public:
    typedef QSharedPointer<WirelessNetwork> Ptr;
    typedef QList<Ptr> List;
    ~WirelessNetwork() override;
    /**
     * ESSID of the network
     */
    QString ssid() const;

    /**
     * Signal strength of the network.  Syntactic sugar around tracking the reference access
     * point and watching its signal strength
     */
    int signalStrength() const;

    /**
     * The uni of the current 'best' (strongest) Access Point. Note that this may change or disappear over time.
     * Get the Access Point object using @ref WirelessDevice::findAccessPoint() on the NetworkInterface this network was obtained from.
     * Use @ref WirelessDevice::accessPointDisappeared() or
     * WirelessNetwork::referenceAccessPointChanged() to detect this.
     */
    AccessPoint::Ptr referenceAccessPoint() const;

    /**
     * List of access points
     * @warning Subject to change, do not store!
     */
    AccessPoint::List accessPoints() const;

    /**
     * The uni of device associated with this network.
     */
    QString device() const;

Q_SIGNALS:
    /**
     * Indicate that the signal strength changed
     * @param strength strength as a percentage.
     */
    void signalStrengthChanged(int strength);
    /**
     * Indicate that the reference access point changed
     * @param apUni new access point or empty string if none
     */
    void referenceAccessPointChanged(const QString &apUni);
    /**
     * Indicate that this network has no more access points
     * (meaning the network has disappeared from view of the network interface)
     * @param ssid the SSID of this network
     */
    void disappeared(const QString &ssid);

private:
    Q_DECLARE_PRIVATE(WirelessNetwork)
    Q_PRIVATE_SLOT(d_func(), void accessPointAppeared(const QString &))
    Q_PRIVATE_SLOT(d_func(), void accessPointDisappeared(const QString &))
    Q_PRIVATE_SLOT(d_func(), void updateStrength())

    WirelessNetworkPrivate *const d_ptr;

    NETWORKMANAGERQT_NO_EXPORT explicit WirelessNetwork(const AccessPoint::Ptr &accessPoint, WirelessDevice *device);
};

}
#endif // NETWORKMANAGERQT_WIRELESSNETWORK_H
