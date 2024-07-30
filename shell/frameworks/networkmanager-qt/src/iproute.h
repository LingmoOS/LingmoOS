/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_IPROUTE_H
#define NETWORKMANAGERQT_IPROUTE_H

#include "ipaddress.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QNetworkAddressEntry>

namespace NetworkManager
{
/**
 * This class represents IP route
 */
class NETWORKMANAGERQT_EXPORT IpRoute : public QNetworkAddressEntry
{
public:
    /**
     * Constructs an empty IpRoute object.
     */
    IpRoute();

    /**
     * Constructs an IpRoute object that is a copy of the object @p other.
     */
    IpRoute(const IpRoute &other);

    /**
     * Destroys this IpRoute object.
     */
    ~IpRoute();

    /**
     * Returns true if the route IP is defined.
     */
    bool isValid() const;

    /**
     * Defines the next hop of the given route.
     */
    void setNextHop(const QHostAddress &nextHop) const;

    /**
     * Returns the next hop of the given route.
     */
    QHostAddress nextHop() const;

    /**
     * Defines the @p metric of the given route,
     * lower values have higher priority.
     */
    void setMetric(quint32 metric);

    /**
     * Returns the route metric number of the given route.
     */
    quint32 metric() const;

    /**
     * Makes a copy of the IpRoute object @p other.
     */
    IpRoute &operator=(const IpRoute &other);

private:
    class Private;
    Private *const d;
};
typedef QList<IpRoute> IpRoutes;

} // namespace NetworkManager

#endif // NETWORKMANAGERQT_IPROUTE_H
