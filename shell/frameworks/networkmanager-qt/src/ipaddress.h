/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_IPADDRESS_H
#define NETWORKMANAGERQT_IPADDRESS_H

#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QNetworkAddressEntry>

namespace NetworkManager
{
/**
 * This class represents IP address
 */
class NETWORKMANAGERQT_EXPORT IpAddress : public QNetworkAddressEntry
{
public:
    /**
     * Constructs an empty IpAddress object.
     */
    IpAddress();

    /**
     * Constructs an IpAddress object that is a copy of the object other.
     */
    IpAddress(const IpAddress &other);

    /**
     * Destroys this IpAddress object.
     */
    ~IpAddress();

    /**
     * Return if the IP address is defined.
     */
    bool isValid() const;

    /**
     * Defines the default @p gateway of this object.
     */
    void setGateway(const QHostAddress &gateway);

    /**
     * Returns the default gateway of this object.
     */
    QHostAddress gateway() const;

    /**
     * Makes a copy of the IpAddress object @p other.
     */
    IpAddress &operator=(const IpAddress &other);

private:
    class Private;
    Private *const d;
};
typedef QList<IpAddress> IpAddresses;

} // namespace NetworkManager

#endif // NETWORKMANAGERQT_IPADDRESS_H
