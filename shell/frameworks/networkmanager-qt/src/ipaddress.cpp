/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ipaddress.h"

namespace NetworkManager
{
class IpAddress::Private
{
public:
    QHostAddress gateway;
};

}

NetworkManager::IpAddress::IpAddress()
    : d(new Private)
{
}

NetworkManager::IpAddress::~IpAddress()
{
    delete d;
}

NetworkManager::IpAddress::IpAddress(const NetworkManager::IpAddress &other)
    : QNetworkAddressEntry(other)
    , d(new Private)
{
    *this = other;
}

bool NetworkManager::IpAddress::isValid() const
{
    return !ip().isNull();
}

void NetworkManager::IpAddress::setGateway(const QHostAddress &gateway)
{
    d->gateway = gateway;
}

QHostAddress NetworkManager::IpAddress::gateway() const
{
    return d->gateway;
}

NetworkManager::IpAddress &NetworkManager::IpAddress::operator=(const NetworkManager::IpAddress &other)
{
    if (this == &other) {
        return *this;
    }

    QNetworkAddressEntry::operator=(other);
    *d = *other.d;

    return *this;
}
