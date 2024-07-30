/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ipv4setting.h"
#include "ipv4setting_p.h"

#include "generictypes.h"

#include <arpa/inet.h>

#include <QDebug>

NetworkManager::Ipv4SettingPrivate::Ipv4SettingPrivate()
    : name(NMQT_SETTING_IP4_CONFIG_SETTING_NAME)
    , method(NetworkManager::Ipv4Setting::Automatic)
    , routeMetric(-1)
    , ignoreAutoRoutes(false)
    , ignoreAutoDns(false)
    , dhcpSendHostname(true)
    , neverDefault(false)
    , mayFail(true)
    , dadTimeout(-1)
    , dnsPriority(0)
{
}

NetworkManager::Ipv4Setting::Ipv4Setting()
    : Setting(Setting::Ipv4)
    , d_ptr(new Ipv4SettingPrivate())
{
}

NetworkManager::Ipv4Setting::Ipv4Setting(const Ptr &other)
    : Setting(other)
    , d_ptr(new Ipv4SettingPrivate())
{
    setMethod(other->method());
    setDns(other->dns());
    setDnsSearch(other->dnsSearch());
    setAddresses(other->addresses());
    setRoutes(other->routes());
    setRouteMetric(other->routeMetric());
    setIgnoreAutoRoutes(other->ignoreAutoRoutes());
    setIgnoreAutoDns(other->ignoreAutoDns());
    setDhcpClientId(other->dhcpClientId());
    setDhcpSendHostname(other->dhcpSendHostname());
    setDhcpHostname(other->dhcpHostname());
    setNeverDefault(other->neverDefault());
    setMayFail(other->mayFail());
    setDadTimeout(other->dadTimeout());
    setDhcpFqdn(other->dhcpFqdn());
    setDnsOptions(other->dnsOptions());
    setDnsPriority(other->dnsPriority());
    setAddressData(other->addressData());
    setRouteData(other->routeData());
}

NetworkManager::Ipv4Setting::~Ipv4Setting()
{
    delete d_ptr;
}

QString NetworkManager::Ipv4Setting::name() const
{
    Q_D(const Ipv4Setting);

    return d->name;
}

void NetworkManager::Ipv4Setting::setMethod(NetworkManager::Ipv4Setting::ConfigMethod type)
{
    Q_D(Ipv4Setting);

    d->method = type;
}

NetworkManager::Ipv4Setting::ConfigMethod NetworkManager::Ipv4Setting::method() const
{
    Q_D(const Ipv4Setting);

    return d->method;
}

void NetworkManager::Ipv4Setting::setDns(const QList<QHostAddress> &dns)
{
    Q_D(Ipv4Setting);

    d->dns = dns;
}

QList<QHostAddress> NetworkManager::Ipv4Setting::dns() const
{
    Q_D(const Ipv4Setting);

    return d->dns;
}

void NetworkManager::Ipv4Setting::setDnsSearch(const QStringList &domains)
{
    Q_D(Ipv4Setting);

    d->dnsSearch = domains;
}

QStringList NetworkManager::Ipv4Setting::dnsSearch() const
{
    Q_D(const Ipv4Setting);

    return d->dnsSearch;
}

void NetworkManager::Ipv4Setting::setAddresses(const QList<IpAddress> &ipv4addresses)
{
    Q_D(Ipv4Setting);

    d->addresses = ipv4addresses;
}

QList<NetworkManager::IpAddress> NetworkManager::Ipv4Setting::addresses() const
{
    Q_D(const Ipv4Setting);

    return d->addresses;
}

void NetworkManager::Ipv4Setting::setRoutes(const QList<IpRoute> &ipv4routes)
{
    Q_D(Ipv4Setting);

    d->routes = ipv4routes;
}

QList<NetworkManager::IpRoute> NetworkManager::Ipv4Setting::routes() const
{
    Q_D(const Ipv4Setting);

    return d->routes;
}

void NetworkManager::Ipv4Setting::setRouteMetric(int metric)
{
    Q_D(Ipv4Setting);

    d->routeMetric = metric;
}

int NetworkManager::Ipv4Setting::routeMetric() const
{
    Q_D(const Ipv4Setting);

    return d->routeMetric;
}

void NetworkManager::Ipv4Setting::setIgnoreAutoRoutes(bool ignore)
{
    Q_D(Ipv4Setting);

    d->ignoreAutoRoutes = ignore;
}

bool NetworkManager::Ipv4Setting::ignoreAutoRoutes() const
{
    Q_D(const Ipv4Setting);

    return d->ignoreAutoRoutes;
}

void NetworkManager::Ipv4Setting::setIgnoreAutoDns(bool ignore)
{
    Q_D(Ipv4Setting);

    d->ignoreAutoDns = ignore;
}

bool NetworkManager::Ipv4Setting::ignoreAutoDns() const
{
    Q_D(const Ipv4Setting);

    return d->ignoreAutoDns;
}

void NetworkManager::Ipv4Setting::setDhcpClientId(const QString &id)
{
    Q_D(Ipv4Setting);

    d->dhcpClientId = id;
}

QString NetworkManager::Ipv4Setting::dhcpClientId() const
{
    Q_D(const Ipv4Setting);

    return d->dhcpClientId;
}

void NetworkManager::Ipv4Setting::setDhcpSendHostname(bool send)
{
    Q_D(Ipv4Setting);

    d->dhcpSendHostname = send;
}

bool NetworkManager::Ipv4Setting::dhcpSendHostname() const
{
    Q_D(const Ipv4Setting);

    return d->dhcpSendHostname;
}

void NetworkManager::Ipv4Setting::setDhcpHostname(const QString &hostname)
{
    Q_D(Ipv4Setting);

    d->dhcpHostname = hostname;
}

QString NetworkManager::Ipv4Setting::dhcpHostname() const
{
    Q_D(const Ipv4Setting);

    return d->dhcpHostname;
}

void NetworkManager::Ipv4Setting::setNeverDefault(bool neverDefault)
{
    Q_D(Ipv4Setting);

    d->neverDefault = neverDefault;
}

bool NetworkManager::Ipv4Setting::neverDefault() const
{
    Q_D(const Ipv4Setting);

    return d->neverDefault;
}

void NetworkManager::Ipv4Setting::setMayFail(bool mayFail)
{
    Q_D(Ipv4Setting);

    d->mayFail = mayFail;
}

bool NetworkManager::Ipv4Setting::mayFail() const
{
    Q_D(const Ipv4Setting);

    return d->mayFail;
}

void NetworkManager::Ipv4Setting::setDadTimeout(qint32 timeout)
{
    Q_D(Ipv4Setting);

    d->dadTimeout = timeout;
}

qint32 NetworkManager::Ipv4Setting::dadTimeout() const
{
    Q_D(const Ipv4Setting);

    return d->dadTimeout;
}

void NetworkManager::Ipv4Setting::setDhcpFqdn(const QString &fqdn)
{
    Q_D(Ipv4Setting);

    d->dhcpFqdn = fqdn;
}

QString NetworkManager::Ipv4Setting::dhcpFqdn() const
{
    Q_D(const Ipv4Setting);

    return d->dhcpFqdn;
}

void NetworkManager::Ipv4Setting::setDnsOptions(const QStringList &options)
{
    Q_D(Ipv4Setting);

    d->dnsOptions = options;
}

QStringList NetworkManager::Ipv4Setting::dnsOptions() const
{
    Q_D(const Ipv4Setting);

    return d->dnsOptions;
}

void NetworkManager::Ipv4Setting::setDnsPriority(qint32 priority)
{
    Q_D(Ipv4Setting);

    d->dnsPriority = priority;
}

qint32 NetworkManager::Ipv4Setting::dnsPriority() const
{
    Q_D(const Ipv4Setting);

    return d->dnsPriority;
}

void NetworkManager::Ipv4Setting::setGateway(const QString &gateway)
{
    Q_D(Ipv4Setting);

    d->gateway = gateway;
}

QString NetworkManager::Ipv4Setting::gateway() const
{
    Q_D(const Ipv4Setting);

    return d->gateway;
}

void NetworkManager::Ipv4Setting::setAddressData(const NMVariantMapList &addressData)
{
    Q_D(Ipv4Setting);

    d->addressData = addressData;
}

NMVariantMapList NetworkManager::Ipv4Setting::addressData() const
{
    Q_D(const Ipv4Setting);

    return d->addressData;
}

void NetworkManager::Ipv4Setting::setRouteData(const NMVariantMapList &routeData)
{
    Q_D(Ipv4Setting);

    d->routeData = routeData;
}

NMVariantMapList NetworkManager::Ipv4Setting::routeData() const
{
    Q_D(const Ipv4Setting);

    return d->routeData;
}

void NetworkManager::Ipv4Setting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NMQT_SETTING_IP4_CONFIG_METHOD))) {
        const QString methodType = setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_METHOD)).toString();

        if (methodType.toLower() == QLatin1String(NMQT_SETTING_IP4_CONFIG_METHOD_AUTO)) {
            setMethod(Automatic);
        } else if (methodType.toLower() == QLatin1String(NMQT_SETTING_IP4_CONFIG_METHOD_LINK_LOCAL)) {
            setMethod(LinkLocal);
        } else if (methodType.toLower() == QLatin1String(NMQT_SETTING_IP4_CONFIG_METHOD_MANUAL)) {
            setMethod(Manual);
        } else if (methodType.toLower() == QLatin1String(NMQT_SETTING_IP4_CONFIG_METHOD_SHARED)) {
            setMethod(Shared);
        } else if (methodType.toLower() == QLatin1String(NMQT_SETTING_IP4_CONFIG_METHOD_DISABLED)) {
            setMethod(Disabled);
        } else {
            setMethod(Automatic);
        }
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP4_CONFIG_DNS))) {
        QList<QHostAddress> dbusDns;
        QList<uint> temp;
        if (setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_DNS)).canConvert<QDBusArgument>()) {
            QDBusArgument dnsArg = setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_DNS)).value<QDBusArgument>();
            temp = qdbus_cast<QList<uint>>(dnsArg);
        } else {
            temp = setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_DNS)).value<QList<uint>>();
        }

        for (const uint utmp : std::as_const(temp)) {
            QHostAddress tmpHost(ntohl(utmp));
            dbusDns << tmpHost;
        }

        setDns(dbusDns);
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP4_CONFIG_DNS_SEARCH))) {
        setDnsSearch(setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_DNS_SEARCH)).toStringList());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP4_CONFIG_ADDRESSES))) {
        QList<NetworkManager::IpAddress> addresses;
        QList<QList<uint>> temp;
        if (setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_ADDRESSES)).canConvert<QDBusArgument>()) {
            QDBusArgument addressArg = setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_ADDRESSES)).value<QDBusArgument>();
            temp = qdbus_cast<QList<QList<uint>>>(addressArg);
        } else {
            temp = setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_ADDRESSES)).value<QList<QList<uint>>>();
        }

        for (const QList<uint> &uintList : std::as_const(temp)) {
            if (uintList.count() != 3) {
                continue;
            }

            NetworkManager::IpAddress address;
            address.setIp(QHostAddress(ntohl(uintList.at(0))));
            address.setPrefixLength(uintList.at(1));
            address.setGateway(QHostAddress(ntohl(uintList.at(2))));
            if (!address.isValid()) {
                continue;
            }

            addresses << address;
        }

        setAddresses(addresses);
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP4_CONFIG_ROUTES))) {
        QList<NetworkManager::IpRoute> routes;
        QList<QList<uint>> temp;
        if (setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_ROUTES)).canConvert<QDBusArgument>()) {
            QDBusArgument routeArg = setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_ROUTES)).value<QDBusArgument>();
            temp = qdbus_cast<QList<QList<uint>>>(routeArg);
        } else {
            temp = setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_ROUTES)).value<QList<QList<uint>>>();
        }

        for (const QList<uint> &uintList : std::as_const(temp)) {
            if (uintList.count() != 4) {
                continue;
            }

            NetworkManager::IpRoute route;
            route.setIp(QHostAddress(ntohl(uintList.at(0))));
            route.setPrefixLength(uintList.at(1));
            route.setNextHop(QHostAddress(ntohl(uintList.at(2))));
            route.setMetric((quint32)uintList.at(3));
            if (!route.isValid()) {
                continue;
            }
            routes << route;
        }

        if (!routes.isEmpty()) {
            setRoutes(routes);
        }
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP4_CONFIG_ROUTE_METRIC))) {
        setRouteMetric(setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_ROUTE_METRIC)).toInt());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP4_CONFIG_IGNORE_AUTO_ROUTES))) {
        setIgnoreAutoRoutes(setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_IGNORE_AUTO_ROUTES)).toBool());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP4_CONFIG_IGNORE_AUTO_DNS))) {
        setIgnoreAutoDns(setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_IGNORE_AUTO_DNS)).toBool());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP4_CONFIG_DHCP_CLIENT_ID))) {
        setDhcpClientId(setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_DHCP_CLIENT_ID)).toString());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP4_CONFIG_DHCP_SEND_HOSTNAME))) {
        setDhcpSendHostname(setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_DHCP_SEND_HOSTNAME)).toBool());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP4_CONFIG_DHCP_HOSTNAME))) {
        setDhcpHostname(setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_DHCP_HOSTNAME)).toString());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP4_CONFIG_NEVER_DEFAULT))) {
        setNeverDefault(setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_NEVER_DEFAULT)).toBool());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP4_CONFIG_MAY_FAIL))) {
        setMayFail(setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_MAY_FAIL)).toBool());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP4_CONFIG_DAD_TIMEOUT))) {
        setDadTimeout(setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_DAD_TIMEOUT)).toUInt());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP4_CONFIG_DHCP_FQDN))) {
        setDhcpFqdn(setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_DHCP_FQDN)).toString());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP4_CONFIG_DNS_OPTIONS))) {
        setDnsOptions(setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_DNS_OPTIONS)).toStringList());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP4_CONFIG_DNS_PRIORITY))) {
        setDnsPriority(setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_DNS_PRIORITY)).toInt());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP4_CONFIG_GATEWAY))) {
        setGateway(setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_GATEWAY)).toString());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP4_CONFIG_ROUTE_DATA))) {
        setRouteData(qdbus_cast<NMVariantMapList>(setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_ROUTE_DATA))));
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP4_CONFIG_ADDRESS_DATA))) {
        setAddressData(qdbus_cast<NMVariantMapList>(setting.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_ADDRESS_DATA))));
    }
}

QVariantMap NetworkManager::Ipv4Setting::toMap() const
{
    QVariantMap setting;

    if (method() == Automatic) {
        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_METHOD), QLatin1String(NMQT_SETTING_IP4_CONFIG_METHOD_AUTO));
    } else if (method() == LinkLocal) {
        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_METHOD), QLatin1String(NMQT_SETTING_IP4_CONFIG_METHOD_LINK_LOCAL));
    } else if (method() == Manual) {
        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_METHOD), QLatin1String(NMQT_SETTING_IP4_CONFIG_METHOD_MANUAL));
    } else if (method() == Shared) {
        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_METHOD), QLatin1String(NMQT_SETTING_IP4_CONFIG_METHOD_SHARED));
    } else if (method() == Disabled) {
        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_METHOD), QLatin1String(NMQT_SETTING_IP4_CONFIG_METHOD_DISABLED));
    }

    if (!dns().isEmpty()) {
        QList<uint> dbusDns;
        const QList<QHostAddress> dnsList = dns();
        for (const QHostAddress &dns : dnsList) {
            dbusDns << htonl(dns.toIPv4Address());
        }

        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_DNS), QVariant::fromValue(dbusDns));
    }

    if (!dnsSearch().isEmpty()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_DNS_SEARCH), dnsSearch());
    }

    if (!addresses().isEmpty()) {
        QList<QList<uint>> dbusAddresses;
        const auto addressesList = addresses();
        for (const NetworkManager::IpAddress &addr : addressesList) {
            QList<uint> dbusAddress;
            dbusAddress << htonl(addr.ip().toIPv4Address()) << addr.prefixLength() << htonl(addr.gateway().toIPv4Address());
            dbusAddresses << dbusAddress;
        }

        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_ADDRESSES), QVariant::fromValue(dbusAddresses));
    }

    if (!routes().isEmpty()) {
        QList<QList<uint>> dbusRoutes;
        const auto routesList = routes();
        for (const NetworkManager::IpRoute &route : routesList) {
            QList<uint> dbusRoute;
            dbusRoute << htonl(route.ip().toIPv4Address()) << route.prefixLength() << htonl(route.nextHop().toIPv4Address()) << route.metric();
            dbusRoutes << dbusRoute;
        }

        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_ROUTES), QVariant::fromValue(dbusRoutes));
    }

    if (routeMetric() >= 0) {
        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_ROUTE_METRIC), routeMetric());
    }

    if (ignoreAutoRoutes()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_IGNORE_AUTO_ROUTES), ignoreAutoRoutes());
    }

    if (ignoreAutoDns()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_IGNORE_AUTO_DNS), ignoreAutoDns());
    }

    if (!dhcpClientId().isEmpty()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_DHCP_CLIENT_ID), dhcpClientId());
    }

    if (!dhcpSendHostname()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_DHCP_SEND_HOSTNAME), dhcpSendHostname());
    }

    if (!dhcpHostname().isEmpty()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_DHCP_HOSTNAME), dhcpHostname());
    }

    if (neverDefault()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_NEVER_DEFAULT), neverDefault());
    }

    if (!mayFail()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_MAY_FAIL), mayFail());
    }

    if (dadTimeout() >= 0) {
        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_DAD_TIMEOUT), dadTimeout());
    }

    if (!dhcpFqdn().isEmpty()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_DHCP_FQDN), dhcpFqdn());
    }

    if (!dnsOptions().isEmpty()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_DNS_OPTIONS), dnsOptions());
    }

    if (dnsPriority() != 0) {
        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_DNS_PRIORITY), dnsPriority());
    }

    if (!gateway().isEmpty()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_GATEWAY), gateway());
    }

    if (!addressData().empty()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_ADDRESS_DATA), QVariant::fromValue(addressData()));
    }

    if (!routeData().empty()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_ROUTE_DATA), QVariant::fromValue(routeData()));
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::Ipv4Setting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NMQT_SETTING_IP4_CONFIG_METHOD << ": " << setting.method() << '\n';
    dbg.nospace() << NMQT_SETTING_IP4_CONFIG_DNS << ":\n";
    const QList<QHostAddress> dnsList = setting.dns();
    for (const QHostAddress &address : dnsList) {
        dbg.nospace() << address.toString() << '\n';
    }
    dbg.nospace() << NMQT_SETTING_IP4_CONFIG_DNS_SEARCH << ": " << setting.dnsSearch() << '\n';
    dbg.nospace() << NMQT_SETTING_IP4_CONFIG_ADDRESSES << '\n';
    const auto addressesList = setting.addresses();
    for (const NetworkManager::IpAddress &address : addressesList) {
        dbg.nospace() << address.ip() << ": " << address.gateway() << ": " << address.netmask() << '\n';
    }
    dbg.nospace() << NMQT_SETTING_IP4_CONFIG_ROUTES << '\n';
    const auto routesList = setting.routes();
    for (const NetworkManager::IpRoute &route : routesList) {
        dbg.nospace() << route.ip() << ": " << route.netmask() << ": " << route.nextHop() << ": " << route.metric() << '\n';
    }
    dbg.nospace() << NMQT_SETTING_IP4_CONFIG_ROUTE_METRIC << ":" << setting.routeMetric() << '\n';
    dbg.nospace() << NMQT_SETTING_IP4_CONFIG_IGNORE_AUTO_ROUTES << ": " << setting.ignoreAutoRoutes() << '\n';
    dbg.nospace() << NMQT_SETTING_IP4_CONFIG_IGNORE_AUTO_DNS << ": " << setting.ignoreAutoDns() << '\n';
    dbg.nospace() << NMQT_SETTING_IP4_CONFIG_DHCP_CLIENT_ID << ": " << setting.dhcpClientId() << '\n';
    dbg.nospace() << NMQT_SETTING_IP4_CONFIG_DHCP_SEND_HOSTNAME << ": " << setting.dhcpSendHostname() << '\n';
    dbg.nospace() << NMQT_SETTING_IP4_CONFIG_DHCP_HOSTNAME << ": " << setting.dhcpHostname() << '\n';
    dbg.nospace() << NMQT_SETTING_IP4_CONFIG_NEVER_DEFAULT << ": " << setting.neverDefault() << '\n';
    dbg.nospace() << NMQT_SETTING_IP4_CONFIG_MAY_FAIL << ": " << setting.mayFail() << '\n';
    dbg.nospace() << NMQT_SETTING_IP4_CONFIG_DAD_TIMEOUT << ": " << setting.dadTimeout() << '\n';
    dbg.nospace() << NMQT_SETTING_IP4_CONFIG_DHCP_FQDN << ": " << setting.dhcpFqdn() << '\n';
    dbg.nospace() << NMQT_SETTING_IP4_CONFIG_DNS_OPTIONS << ": " << setting.dnsOptions() << '\n';
    dbg.nospace() << NMQT_SETTING_IP4_CONFIG_DNS_PRIORITY << ": " << setting.dnsPriority() << '\n';
    dbg.nospace() << NMQT_SETTING_IP4_CONFIG_GATEWAY << ": " << setting.gateway() << '\n';
    dbg.nospace() << NMQT_SETTING_IP4_CONFIG_ADDRESS_DATA << ": " << '\n';
    const NMVariantMapList addDataList = setting.addressData();
    for (const QVariantMap &addressData : addDataList) {
        QVariantMap::const_iterator i = addressData.constBegin();
        while (i != addressData.constEnd()) {
            dbg.nospace() << i.key() << ": " << i.value() << '\n';
        }
    }
    dbg.nospace() << NMQT_SETTING_IP4_CONFIG_ROUTE_DATA << ": " << '\n';
    const NMVariantMapList routeDataList = setting.routeData();
    for (const QVariantMap &routeData : routeDataList) {
        QVariantMap::const_iterator i = routeData.constBegin();
        while (i != routeData.constEnd()) {
            dbg.nospace() << i.key() << ": " << i.value() << '\n';
        }
    }

    return dbg.maybeSpace();
}
