/*
    SPDX-FileCopyrightText: 2018 Aleksander Morgado <aleksander@aleksander.es>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ipconfig.h"

#include <arpa/inet.h>

#include "dnsdomain.h"

namespace NetworkManager
{
class NetworkManager::DnsDomain::Private
{
public:
    Private(const QString &theName, const QList<QHostAddress> &theServers, const QStringList &theOptions)
        : name(theName)
        , servers(theServers)
        , options(theOptions)
    {
    }
    Private()
    {
    }
    QString name;
    QList<QHostAddress> servers;
    QStringList options;
};

}

NetworkManager::DnsDomain::DnsDomain(const QString &name, const QList<QHostAddress> &servers, const QStringList &options)
    : d(new Private(name, servers, options))
{
}

NetworkManager::DnsDomain::DnsDomain()
    : d(new Private())
{
}

NetworkManager::DnsDomain::DnsDomain(const DnsDomain &other)
    : d(new Private)
{
    *this = other;
}

NetworkManager::DnsDomain::~DnsDomain()
{
    delete d;
}

QString NetworkManager::DnsDomain::name() const
{
    return d->name;
}

void NetworkManager::DnsDomain::setName(const QString &name)
{
    d->name = name;
}

QList<QHostAddress> NetworkManager::DnsDomain::servers() const
{
    return d->servers;
}

void NetworkManager::DnsDomain::setServers(const QList<QHostAddress> &servers)
{
    d->servers = servers;
}

QStringList NetworkManager::DnsDomain::options() const
{
    return d->options;
}

void NetworkManager::DnsDomain::setOptions(const QStringList &options)
{
    d->options = options;
}

NetworkManager::DnsDomain &NetworkManager::DnsDomain::operator=(const DnsDomain &other)
{
    if (this == &other) {
        return *this;
    }

    *d = *other.d;
    return *this;
}
