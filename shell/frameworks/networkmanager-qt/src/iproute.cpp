/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ipconfig.h"

namespace NetworkManager
{
class NetworkManager::IpRoute::Private
{
public:
    Private()
        : metric(0)
    {
    }
    QHostAddress nextHop;
    quint32 metric;
};

}

NetworkManager::IpRoute::IpRoute()
    : d(new Private)
{
}

NetworkManager::IpRoute::~IpRoute()
{
    delete d;
}

NetworkManager::IpRoute::IpRoute(const NetworkManager::IpRoute &other)
    : QNetworkAddressEntry(other)
    , d(new Private)
{
    *this = other;
}

void NetworkManager::IpRoute::setNextHop(const QHostAddress &nextHop) const
{
    d->nextHop = nextHop;
}

QHostAddress NetworkManager::IpRoute::nextHop() const
{
    return d->nextHop;
}

void NetworkManager::IpRoute::setMetric(quint32 metric)
{
    d->metric = metric;
}

quint32 NetworkManager::IpRoute::metric() const
{
    return d->metric;
}

NetworkManager::IpRoute &NetworkManager::IpRoute::operator=(const NetworkManager::IpRoute &other)
{
    if (this == &other) {
        return *this;
    }

    QNetworkAddressEntry::operator=(other);
    *d = *other.d;

    return *this;
}

bool NetworkManager::IpRoute::isValid() const
{
    return !ip().isNull();
}
