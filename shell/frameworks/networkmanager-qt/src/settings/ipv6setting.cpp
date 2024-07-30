/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ipv6setting.h"
#include "ipv6setting_p.h"

#include "../ipconfig.h"
#include "generictypes.h"
#include "utils.h"

#include <arpa/inet.h>

NetworkManager::Ipv6SettingPrivate::Ipv6SettingPrivate()
    : name(NMQT_SETTING_IP6_CONFIG_SETTING_NAME)
    , method(NetworkManager::Ipv6Setting::Automatic)
    , routeMetric(-1)
    , ignoreAutoRoutes(false)
    , ignoreAutoDns(false)
    , neverDefault(false)
    , mayFail(true)
    , privacy(NetworkManager::Ipv6Setting::Unknown)
    , dadTimeout(-1)
    , addressGenMode(NetworkManager::Ipv6Setting::StablePrivacy)
    , dhcpTimeout(0)
    , routeTable(0)
{
}

NetworkManager::Ipv6Setting::Ipv6Setting()
    : Setting(Setting::Ipv6)
    , d_ptr(new Ipv6SettingPrivate())
{
}

NetworkManager::Ipv6Setting::Ipv6Setting(const Ptr &other)
    : Setting(other)
    , d_ptr(new Ipv6SettingPrivate())
{
    setMethod(other->method());
    setDns(other->dns());
    setDnsSearch(other->dnsSearch());
    setAddresses(other->addresses());
    setRoutes(other->routes());
    setRouteMetric(other->routeMetric());
    setIgnoreAutoRoutes(other->ignoreAutoRoutes());
    setIgnoreAutoDns(other->ignoreAutoDns());
    setNeverDefault(other->neverDefault());
    setMayFail(other->mayFail());
    setDadTimeout(other->dadTimeout());
    setAddressGenMode(other->addressGenMode());
    setDhcpTimeout(other->dhcpTimeout());
    setDhcpHostname(other->dhcpHostname());
    setDhcpDuid(other->dhcpDuid());
    setToken(other->token());
    setDnsOptions(other->dnsOptions());
    setAddressData(other->addressData());
    setRouteData(other->routeData());
    setRouteTable(other->routeTable());
}

NetworkManager::Ipv6Setting::~Ipv6Setting()
{
    delete d_ptr;
}

QString NetworkManager::Ipv6Setting::name() const
{
    Q_D(const Ipv6Setting);

    return d->name;
}

void NetworkManager::Ipv6Setting::setMethod(NetworkManager::Ipv6Setting::ConfigMethod type)
{
    Q_D(Ipv6Setting);

    d->method = type;
}

NetworkManager::Ipv6Setting::ConfigMethod NetworkManager::Ipv6Setting::method() const
{
    Q_D(const Ipv6Setting);

    return d->method;
}

void NetworkManager::Ipv6Setting::setDns(const QList<QHostAddress> &dns)
{
    Q_D(Ipv6Setting);

    d->dns = dns;
}

QList<QHostAddress> NetworkManager::Ipv6Setting::dns() const
{
    Q_D(const Ipv6Setting);

    return d->dns;
}

void NetworkManager::Ipv6Setting::setDnsSearch(const QStringList &domains)
{
    Q_D(Ipv6Setting);

    d->dnsSearch = domains;
}

QStringList NetworkManager::Ipv6Setting::dnsSearch() const
{
    Q_D(const Ipv6Setting);

    return d->dnsSearch;
}

void NetworkManager::Ipv6Setting::setAddresses(const QList<IpAddress> ipv6addresses)
{
    Q_D(Ipv6Setting);

    d->addresses = ipv6addresses;
}

QList<NetworkManager::IpAddress> NetworkManager::Ipv6Setting::addresses() const
{
    Q_D(const Ipv6Setting);

    return d->addresses;
}

void NetworkManager::Ipv6Setting::setRoutes(const QList<NetworkManager::IpRoute> ipv6routes)
{
    Q_D(Ipv6Setting);

    d->routes = ipv6routes;
}

QList<NetworkManager::IpRoute> NetworkManager::Ipv6Setting::routes() const
{
    Q_D(const Ipv6Setting);

    return d->routes;
}

void NetworkManager::Ipv6Setting::setRouteMetric(int metric)
{
    Q_D(Ipv6Setting);

    d->routeMetric = metric;
}

int NetworkManager::Ipv6Setting::routeMetric() const
{
    Q_D(const Ipv6Setting);

    return d->routeMetric;
}

void NetworkManager::Ipv6Setting::setIgnoreAutoRoutes(bool ignore)
{
    Q_D(Ipv6Setting);

    d->ignoreAutoRoutes = ignore;
}

bool NetworkManager::Ipv6Setting::ignoreAutoRoutes() const
{
    Q_D(const Ipv6Setting);

    return d->ignoreAutoRoutes;
}

void NetworkManager::Ipv6Setting::setIgnoreAutoDns(bool ignore)
{
    Q_D(Ipv6Setting);

    d->ignoreAutoDns = ignore;
}

bool NetworkManager::Ipv6Setting::ignoreAutoDns() const
{
    Q_D(const Ipv6Setting);

    return d->ignoreAutoDns;
}

void NetworkManager::Ipv6Setting::setNeverDefault(bool neverDefault)
{
    Q_D(Ipv6Setting);

    d->neverDefault = neverDefault;
}

bool NetworkManager::Ipv6Setting::neverDefault() const
{
    Q_D(const Ipv6Setting);

    return d->neverDefault;
}

void NetworkManager::Ipv6Setting::setMayFail(bool mayFail)
{
    Q_D(Ipv6Setting);

    d->mayFail = mayFail;
}

bool NetworkManager::Ipv6Setting::mayFail() const
{
    Q_D(const Ipv6Setting);

    return d->mayFail;
}

void NetworkManager::Ipv6Setting::setPrivacy(IPv6Privacy privacy)
{
    Q_D(Ipv6Setting);

    d->privacy = privacy;
}

NetworkManager::Ipv6Setting::IPv6Privacy NetworkManager::Ipv6Setting::privacy() const
{
    Q_D(const Ipv6Setting);

    return d->privacy;
}

void NetworkManager::Ipv6Setting::setDadTimeout(qint32 timeout)
{
    Q_D(Ipv6Setting);

    d->dadTimeout = timeout;
}

qint32 NetworkManager::Ipv6Setting::dadTimeout() const
{
    Q_D(const Ipv6Setting);

    return d->dadTimeout;
}

void NetworkManager::Ipv6Setting::setDhcpTimeout(qint32 timeout)
{
    Q_D(Ipv6Setting);

    d->dhcpTimeout = timeout;
}

qint32 NetworkManager::Ipv6Setting::dhcpTimeout() const
{
    Q_D(const Ipv6Setting);

    return d->dhcpTimeout;
}

void NetworkManager::Ipv6Setting::setDhcpHostname(const QString &hostname)
{
    Q_D(Ipv6Setting);

    d->dhcpHostname = hostname;
}

QString NetworkManager::Ipv6Setting::dhcpHostname() const
{
    Q_D(const Ipv6Setting);

    return d->dhcpHostname;
}

void NetworkManager::Ipv6Setting::setDhcpDuid(const QString &duid)
{
    Q_D(Ipv6Setting);

    d->dhcpDuid = duid;
}

QString NetworkManager::Ipv6Setting::dhcpDuid() const
{
    Q_D(const Ipv6Setting);

    return d->dhcpDuid;
}

void NetworkManager::Ipv6Setting::setDnsOptions(const QStringList &options)
{
    Q_D(Ipv6Setting);

    d->dnsOptions = options;
}

QStringList NetworkManager::Ipv6Setting::dnsOptions() const
{
    Q_D(const Ipv6Setting);

    return d->dnsOptions;
}

void NetworkManager::Ipv6Setting::setAddressData(const NMVariantMapList &addressData)
{
    Q_D(Ipv6Setting);

    d->addressData = addressData;
}

NMVariantMapList NetworkManager::Ipv6Setting::addressData() const
{
    Q_D(const Ipv6Setting);

    return d->addressData;
}

void NetworkManager::Ipv6Setting::setAddressGenMode(IPv6AddressGenMode mode)
{
    Q_D(Ipv6Setting);

    d->addressGenMode = mode;
}

NetworkManager::Ipv6Setting::IPv6AddressGenMode NetworkManager::Ipv6Setting::addressGenMode() const
{
    Q_D(const Ipv6Setting);

    return d->addressGenMode;
}

void NetworkManager::Ipv6Setting::setRouteData(const NMVariantMapList &routeData)
{
    Q_D(Ipv6Setting);

    d->routeData = routeData;
}

NMVariantMapList NetworkManager::Ipv6Setting::routeData() const
{
    Q_D(const Ipv6Setting);

    return d->routeData;
}

void NetworkManager::Ipv6Setting::setToken(const QString &token)
{
    Q_D(Ipv6Setting);

    d->token = token;
}

QString NetworkManager::Ipv6Setting::token() const
{
    Q_D(const Ipv6Setting);

    return d->token;
}

void NetworkManager::Ipv6Setting::setRouteTable(quint32 routeTable)
{
    Q_D(Ipv6Setting);

    d->routeTable = routeTable;
}

quint32 NetworkManager::Ipv6Setting::routeTable() const
{
    Q_D(const Ipv6Setting);

    return d->routeTable;
}

void NetworkManager::Ipv6Setting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NMQT_SETTING_IP6_CONFIG_METHOD))) {
        const QString methodType = setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_METHOD)).toString();

        if (methodType.toLower() == QLatin1String(NMQT_SETTING_IP6_CONFIG_METHOD_AUTO)) {
            setMethod(Automatic);
        } else if (methodType.toLower() == QLatin1String(NMQT_SETTING_IP6_CONFIG_METHOD_DHCP)) {
            setMethod(Dhcp);
        } else if (methodType.toLower() == QLatin1String(NMQT_SETTING_IP6_CONFIG_METHOD_LINK_LOCAL)) {
            setMethod(LinkLocal);
        } else if (methodType.toLower() == QLatin1String(NMQT_SETTING_IP6_CONFIG_METHOD_MANUAL)) {
            setMethod(Manual);
        } else if (methodType.toLower() == QLatin1String(NMQT_SETTING_IP6_CONFIG_METHOD_IGNORE)) {
            setMethod(Ignored);
        } else if (methodType.toLower() == QLatin1String(NMQT_SETTING_IP6_CONFIG_METHOD_DISABLED)) {
            setMethod(ConfigDisabled);
        } else {
            setMethod(Automatic);
        }
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP6_CONFIG_DNS))) {
        QList<QHostAddress> dbusDns;
        QList<QByteArray> temp;
        if (setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_DNS)).canConvert<QDBusArgument>()) {
            QDBusArgument dnsArg = setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_DNS)).value<QDBusArgument>();
            temp = qdbus_cast<QList<QByteArray>>(dnsArg);
        } else {
            temp = setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_DNS)).value<QList<QByteArray>>();
        }

        for (const QByteArray &utmp : std::as_const(temp)) {
            dbusDns << ipv6AddressAsHostAddress(utmp);
        }

        setDns(dbusDns);
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP6_CONFIG_DNS_SEARCH))) {
        setDnsSearch(setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_DNS_SEARCH)).toStringList());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP6_CONFIG_ADDRESSES))) {
        QList<IpV6DBusAddress> temp;
        if (setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_DNS)).canConvert<QDBusArgument>()) {
            QDBusArgument addressArg = setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_ADDRESSES)).value<QDBusArgument>();
            temp = qdbus_cast<QList<IpV6DBusAddress>>(addressArg);
        } else {
            temp = setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_ADDRESSES)).value<QList<IpV6DBusAddress>>();
        }
        QList<NetworkManager::IpAddress> addresses;

        for (const IpV6DBusAddress &addressMap : std::as_const(temp)) {
            if (addressMap.address.isEmpty() || !addressMap.prefix || addressMap.gateway.isEmpty()) {
                continue;
            }

            NetworkManager::IpAddress address;
            address.setIp(ipv6AddressAsHostAddress(addressMap.address));
            address.setPrefixLength(addressMap.prefix);
            address.setGateway(ipv6AddressAsHostAddress(addressMap.gateway));
            if (!address.isValid()) {
                continue;
            }

            addresses << address;
        }

        setAddresses(addresses);
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP6_CONFIG_ROUTES))) {
        QList<IpV6DBusRoute> temp;
        if (setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_ROUTES)).canConvert<QDBusArgument>()) {
            QDBusArgument routeArg = setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_ROUTES)).value<QDBusArgument>();
            temp = qdbus_cast<QList<IpV6DBusRoute>>(routeArg);
        } else {
            temp = setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_ROUTES)).value<QList<IpV6DBusRoute>>();
        }
        QList<NetworkManager::IpRoute> routes;

        for (const IpV6DBusRoute &routeMap : std::as_const(temp)) {
            if (routeMap.destination.isEmpty() || !routeMap.prefix || routeMap.nexthop.isEmpty() || !routeMap.metric) {
                continue;
            }

            NetworkManager::IpRoute route;
            route.setIp(ipv6AddressAsHostAddress(routeMap.destination));
            route.setPrefixLength(routeMap.prefix);
            route.setNextHop(ipv6AddressAsHostAddress(routeMap.nexthop));
            route.setMetric(routeMap.metric);
            if (!route.isValid()) {
                continue;
            }

            routes << route;
        }
        setRoutes(routes);
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP6_CONFIG_ROUTE_METRIC))) {
        setRouteMetric(setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_ROUTE_METRIC)).toInt());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP6_CONFIG_IGNORE_AUTO_ROUTES))) {
        setIgnoreAutoRoutes(setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_IGNORE_AUTO_ROUTES)).toBool());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP6_CONFIG_IGNORE_AUTO_DNS))) {
        setIgnoreAutoDns(setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_IGNORE_AUTO_DNS)).toBool());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP6_CONFIG_NEVER_DEFAULT))) {
        setNeverDefault(setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_NEVER_DEFAULT)).toBool());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP6_CONFIG_MAY_FAIL))) {
        setMayFail(setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_MAY_FAIL)).toBool());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP6_CONFIG_IP6_PRIVACY))) {
        setPrivacy((IPv6Privacy)setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_IP6_PRIVACY)).toUInt());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP6_CONFIG_DAD_TIMEOUT))) {
        setDadTimeout(setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_DAD_TIMEOUT)).toUInt());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP6_CONFIG_DHCP_TIMEOUT))) {
        setDhcpTimeout(setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_DHCP_TIMEOUT)).toUInt());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP6_CONFIG_ADDRESS_GEN_MODE))) {
        setAddressGenMode(static_cast<IPv6AddressGenMode>(setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_ADDRESS_GEN_MODE)).toUInt()));
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP6_CONFIG_DHCP_HOSTNAME))) {
        setDhcpHostname(setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_DHCP_HOSTNAME)).toString());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP6_CONFIG_DHCP_DUID))) {
        setDhcpDuid(setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_DHCP_DUID)).toString());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP6_CONFIG_TOKEN))) {
        setToken(setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_TOKEN)).toString());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP6_CONFIG_DNS_OPTIONS))) {
        setDnsOptions(setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_DNS_OPTIONS)).toStringList());
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP6_CONFIG_ROUTE_DATA))) {
        setRouteData(qdbus_cast<NMVariantMapList>(setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_ROUTE_DATA))));
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP6_CONFIG_ADDRESS_DATA))) {
        setAddressData(qdbus_cast<NMVariantMapList>(setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_ADDRESS_DATA))));
    }

    if (setting.contains(QLatin1String(NMQT_SETTING_IP6_CONFIG_ROUTE_TABLE))) {
        setRouteTable(setting.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_ROUTE_TABLE)).toUInt());
    }
}

QVariantMap NetworkManager::Ipv6Setting::toMap() const
{
    QVariantMap setting;

    if (method() == Automatic) {
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_METHOD), QLatin1String(NMQT_SETTING_IP6_CONFIG_METHOD_AUTO));
    } else if (method() == Dhcp) {
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_METHOD), QLatin1String(NMQT_SETTING_IP6_CONFIG_METHOD_DHCP));
    } else if (method() == LinkLocal) {
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_METHOD), QLatin1String(NMQT_SETTING_IP6_CONFIG_METHOD_LINK_LOCAL));
    } else if (method() == Manual) {
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_METHOD), QLatin1String(NMQT_SETTING_IP6_CONFIG_METHOD_MANUAL));
    } else if (method() == Ignored) {
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_METHOD), QLatin1String(NMQT_SETTING_IP6_CONFIG_METHOD_IGNORE));
    } else if (method() == ConfigDisabled) {
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_METHOD), QLatin1String(NMQT_SETTING_IP6_CONFIG_METHOD_DISABLED));
    }

    if (!dns().isEmpty()) {
        QList<QByteArray> dbusDns;
        const QList<QHostAddress> dnsList = dns();
        for (const QHostAddress &dns : dnsList) {
            dbusDns << ipv6AddressFromHostAddress(dns);
        }
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_DNS), QVariant::fromValue(dbusDns));
    }

    if (!dnsSearch().isEmpty()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_DNS_SEARCH), dnsSearch());
    }

    if (!addresses().isEmpty()) {
        QList<IpV6DBusAddress> dbusAddresses;
        const QList<NetworkManager::IpAddress> addressesList = addresses();
        for (const NetworkManager::IpAddress &addr : addressesList) {
            IpV6DBusAddress dbusAddress;
            dbusAddress.address = ipv6AddressFromHostAddress(addr.ip());
            dbusAddress.prefix = addr.prefixLength();
            dbusAddress.gateway = ipv6AddressFromHostAddress(addr.gateway());
            dbusAddresses << dbusAddress;
        }

        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_ADDRESSES), QVariant::fromValue(dbusAddresses));
    }

    if (!routes().isEmpty()) {
        QList<IpV6DBusRoute> dbusRoutes;
        const QList<NetworkManager::IpRoute> routesList = routes();
        for (const NetworkManager::IpRoute &route : routesList) {
            IpV6DBusRoute dbusRoute;
            dbusRoute.destination = ipv6AddressFromHostAddress(route.ip());
            dbusRoute.prefix = route.prefixLength();
            dbusRoute.nexthop = ipv6AddressFromHostAddress(route.nextHop());
            dbusRoute.metric = route.metric();
            dbusRoutes << dbusRoute;
        }

        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_ROUTES), QVariant::fromValue(dbusRoutes));
    }

    if (routeMetric() >= 0) {
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_ROUTE_METRIC), routeMetric());
    }

    if (ignoreAutoRoutes()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_IGNORE_AUTO_ROUTES), ignoreAutoRoutes());
    }

    if (ignoreAutoDns()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_IGNORE_AUTO_DNS), ignoreAutoDns());
    }

    if (neverDefault()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_NEVER_DEFAULT), neverDefault());
    }

    if (!mayFail()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_MAY_FAIL), mayFail());
    }

    if (privacy() != Unknown) {
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_IP6_PRIVACY), privacy());
    }

    if (dadTimeout() >= 0) {
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_DAD_TIMEOUT), dadTimeout());
    }

    setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_ADDRESS_GEN_MODE), addressGenMode());

    if (dhcpTimeout() > 0) {
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_DHCP_TIMEOUT), dhcpTimeout());
    }

    if (!dhcpHostname().isEmpty()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_DHCP_HOSTNAME), dhcpHostname());
    }

    if (!dhcpDuid().isEmpty()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_DHCP_DUID), dhcpDuid());
    }

    if (!token().isEmpty()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_TOKEN), token());
    }

    if (!dnsOptions().isEmpty()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_DNS_OPTIONS), dnsOptions());
    }

    if (!addressData().empty()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_ADDRESS_DATA), QVariant::fromValue(addressData()));
    }

    if (!routeData().empty()) {
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_ROUTE_DATA), QVariant::fromValue(routeData()));
    }

    if (routeTable() > 0) {
        setting.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_ROUTE_TABLE), routeTable());
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::Ipv6Setting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NMQT_SETTING_IP6_CONFIG_METHOD << ": " << setting.method() << '\n';
    dbg.nospace() << NMQT_SETTING_IP6_CONFIG_DNS << '\n';
    const QList<QHostAddress> hostAdresses = setting.dns();
    for (const QHostAddress &address : hostAdresses) {
        dbg.nospace() << address.toString() << '\n';
    }
    dbg.nospace() << NMQT_SETTING_IP6_CONFIG_DNS_SEARCH << ": " << setting.dnsSearch() << '\n';
    dbg.nospace() << NMQT_SETTING_IP6_CONFIG_ADDRESSES << '\n';
    const QList<NetworkManager::IpAddress> ipList = setting.addresses();
    for (const NetworkManager::IpAddress &address : ipList) {
        dbg.nospace() << address.ip().toString() << ": " << address.gateway().toString() << ": " << address.netmask() << '\n';
    }
    dbg.nospace() << NMQT_SETTING_IP6_CONFIG_ROUTES << '\n';
    const QList<NetworkManager::IpRoute> routesList = setting.routes();
    for (const NetworkManager::IpRoute &route : routesList) {
        dbg.nospace() << route.ip().toString() << ": " << route.metric() << ": " << route.nextHop().toString() << ": " << route.metric() << '\n';
    }
    dbg.nospace() << NMQT_SETTING_IP6_CONFIG_ROUTE_METRIC << ":" << setting.routeMetric() << '\n';
    dbg.nospace() << NMQT_SETTING_IP6_CONFIG_IGNORE_AUTO_ROUTES << ": " << setting.ignoreAutoRoutes() << '\n';
    dbg.nospace() << NMQT_SETTING_IP6_CONFIG_IGNORE_AUTO_DNS << ": " << setting.ignoreAutoDns() << '\n';
    dbg.nospace() << NMQT_SETTING_IP6_CONFIG_NEVER_DEFAULT << ": " << setting.neverDefault() << '\n';
    dbg.nospace() << NMQT_SETTING_IP6_CONFIG_MAY_FAIL << ": " << setting.mayFail() << '\n';
    dbg.nospace() << NMQT_SETTING_IP6_CONFIG_IP6_PRIVACY << ": " << setting.privacy() << '\n';
    dbg.nospace() << NMQT_SETTING_IP6_CONFIG_DAD_TIMEOUT << ": " << setting.dadTimeout() << '\n';
    dbg.nospace() << NMQT_SETTING_IP6_CONFIG_ADDRESS_GEN_MODE << ": " << setting.addressGenMode() << '\n';
    dbg.nospace() << NMQT_SETTING_IP6_CONFIG_DHCP_TIMEOUT << ": " << setting.dhcpTimeout() << '\n';
    dbg.nospace() << NMQT_SETTING_IP6_CONFIG_DHCP_HOSTNAME << ": " << setting.dhcpHostname() << '\n';
    dbg.nospace() << NMQT_SETTING_IP6_CONFIG_DHCP_DUID << ": " << setting.dhcpDuid() << '\n';
    dbg.nospace() << NMQT_SETTING_IP6_CONFIG_TOKEN << ": " << setting.token() << '\n';
    dbg.nospace() << NMQT_SETTING_IP6_CONFIG_DNS_OPTIONS << ": " << setting.dnsOptions() << '\n';
    dbg.nospace() << NMQT_SETTING_IP6_CONFIG_ADDRESS_DATA << ": " << '\n';
    const auto dataList = setting.addressData();
    for (const QVariantMap &addressData : dataList) {
        QVariantMap::const_iterator i = addressData.constBegin();
        while (i != addressData.constEnd()) {
            dbg.nospace() << i.key() << ": " << i.value() << '\n';
        }
    }
    dbg.nospace() << NMQT_SETTING_IP6_CONFIG_ROUTE_DATA << ": " << '\n';
    const auto routeDataList = setting.routeData();
    for (const QVariantMap &routeData : routeDataList) {
        QVariantMap::const_iterator i = routeData.constBegin();
        while (i != routeData.constEnd()) {
            dbg.nospace() << i.key() << ": " << i.value() << '\n';
        }
    }
    dbg.nospace() << NMQT_SETTING_IP6_CONFIG_ROUTE_TABLE << ": " << setting.routeTable() << '\n';

    return dbg.maybeSpace();
}
