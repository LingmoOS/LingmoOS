/*
    SPDX-FileCopyrightText: 2018 Aleksander Morgado <aleksander@aleksander.es>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ipconfig.h"

#include <arpa/inet.h>

#include "dnsconfiguration.h"

#include <QVariant>

namespace NetworkManager
{
class NetworkManager::DnsConfiguration::Private
{
public:
    Private(const QStringList &theSearches, const QStringList &theOptions, const QList<DnsDomain> theDomains)
        : searches(theSearches)
        , options(theOptions)
        , domains(theDomains)
    {
    }
    Private()
    {
    }
    QStringList searches;
    QStringList options;
    QList<DnsDomain> domains;
};

}

NetworkManager::DnsConfiguration::DnsConfiguration(const QStringList &searches, const QStringList &options, const QList<DnsDomain> domains)
    : d(new Private(searches, options, domains))
{
}

NetworkManager::DnsConfiguration::DnsConfiguration()
    : d(new Private())
{
}

NetworkManager::DnsConfiguration::DnsConfiguration(const DnsConfiguration &other)
    : d(new Private)
{
    *this = other;
}

NetworkManager::DnsConfiguration::~DnsConfiguration()
{
    delete d;
}

QStringList NetworkManager::DnsConfiguration::searches() const
{
    return d->searches;
}

void NetworkManager::DnsConfiguration::setSearches(const QStringList &searches)
{
    d->searches = searches;
}

QStringList NetworkManager::DnsConfiguration::options() const
{
    return d->options;
}

void NetworkManager::DnsConfiguration::setOptions(const QStringList &options)
{
    d->options = options;
}

QList<NetworkManager::DnsDomain> NetworkManager::DnsConfiguration::domains() const
{
    return d->domains;
}

void NetworkManager::DnsConfiguration::setDomains(const QList<NetworkManager::DnsDomain> &domains)
{
    d->domains = domains;
}

QVariantMap NetworkManager::DnsConfiguration::toMap() const
{
    QVariantMap map;

    map["searches"] = d->searches;
    map["options"] = d->options;

    QVariantMap domains;
    for (const NetworkManager::DnsDomain &domain : std::as_const(d->domains)) {
        QVariantMap contents;
        QStringList serversList;
        const QList<QHostAddress> servers = domain.servers();
        for (const QHostAddress &address : servers) {
            serversList.append(address.toString());
        }
        contents["servers"] = serversList;
        contents["options"] = domain.options();
        domains[domain.name()] = contents;
    }
    map["domains"] = domains;

    return map;
}

void NetworkManager::DnsConfiguration::fromMap(const QVariantMap &map)
{
    d->searches = map["searches"].toStringList();
    d->options = map["options"].toStringList();
    d->domains = QList<NetworkManager::DnsDomain>();

    QVariantMap domains = map["domains"].toMap();
    QVariantMap::const_iterator i = domains.constBegin();
    while (i != domains.constEnd()) {
        const QVariantMap contents = i.value().toMap();
        QList<QHostAddress> addressList;
        const QStringList serversList = contents["servers"].toStringList();
        for (const QString &server : serversList) {
            addressList.append(QHostAddress(server));
        }
        NetworkManager::DnsDomain domain(i.key(), addressList, contents["options"].toStringList());
        d->domains.append(domain);
        ++i;
    }
}

NetworkManager::DnsConfiguration &NetworkManager::DnsConfiguration::operator=(const DnsConfiguration &other)
{
    if (this == &other) {
        return *this;
    }

    *d = *other.d;
    return *this;
}
