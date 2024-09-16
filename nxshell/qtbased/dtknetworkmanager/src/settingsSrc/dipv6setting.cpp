// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dipv6setting_p.h"
#include "dnmutils.h"
#include <arpa/inet.h>
#include <QDBusArgument>

DNETWORKMANAGER_BEGIN_NAMESPACE

DIpv6SettingPrivate::DIpv6SettingPrivate()
    : m_ignoreAutoRoutes(false)
    , m_ignoreAutoDns(false)
    , m_neverDefault(false)
    , m_mayFail(true)
    , m_routeMetric(-1)
    , m_method(NMConfigMethod::Automatic)
    , m_routeTable(0)
    , m_dadTimeout(-1)
    , m_dhcpTimeout(0)
    , m_privacy(DIpv6Setting::IPv6Privacy::Unknown)
    , m_addressGenMode(DIpv6Setting::IPv6AddressGenMode::StablePrivacy)
    , m_name(DTK_NM_SETTING_IP6_CONFIG_SETTING_NAME)
{
}

DIpv6Setting::DIpv6Setting()
    : DNMSetting(DNMSetting::SettingType::Ipv6)
    , d_ptr(new DIpv6SettingPrivate())
{
}

DIpv6Setting::DIpv6Setting(const QSharedPointer<DIpv6Setting> &other)
    : DNMSetting(other)
    , d_ptr(new DIpv6SettingPrivate())
{
    setMethod(other->method());
    setDns(other->dns());
    setDnsSearch(other->dnsSearch());
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

QString DIpv6Setting::name() const
{
    Q_D(const DIpv6Setting);

    return d->m_name;
}

void DIpv6Setting::setMethod(NMConfigMethod type)
{
    Q_D(DIpv6Setting);

    d->m_method = type;
}

NMConfigMethod DIpv6Setting::method() const
{
    Q_D(const DIpv6Setting);

    return d->m_method;
}

void DIpv6Setting::setDns(const QList<QHostAddress> &dns)
{
    Q_D(DIpv6Setting);

    d->m_dns = dns;
}

QList<QHostAddress> DIpv6Setting::dns() const
{
    Q_D(const DIpv6Setting);

    return d->m_dns;
}

void DIpv6Setting::setDnsSearch(const QStringList &domains)
{
    Q_D(DIpv6Setting);

    d->m_dnsSearch = domains;
}

QStringList DIpv6Setting::dnsSearch() const
{
    Q_D(const DIpv6Setting);

    return d->m_dnsSearch;
}

void DIpv6Setting::setRouteMetric(int metric)
{
    Q_D(DIpv6Setting);

    d->m_routeMetric = metric;
}

int DIpv6Setting::routeMetric() const
{
    Q_D(const DIpv6Setting);

    return d->m_routeMetric;
}

void DIpv6Setting::setIgnoreAutoRoutes(bool ignore)
{
    Q_D(DIpv6Setting);

    d->m_ignoreAutoRoutes = ignore;
}

bool DIpv6Setting::ignoreAutoRoutes() const
{
    Q_D(const DIpv6Setting);

    return d->m_ignoreAutoRoutes;
}

void DIpv6Setting::setIgnoreAutoDns(bool ignore)
{
    Q_D(DIpv6Setting);

    d->m_ignoreAutoDns = ignore;
}

bool DIpv6Setting::ignoreAutoDns() const
{
    Q_D(const DIpv6Setting);

    return d->m_ignoreAutoDns;
}

void DIpv6Setting::setNeverDefault(bool neverDefault)
{
    Q_D(DIpv6Setting);

    d->m_neverDefault = neverDefault;
}

bool DIpv6Setting::neverDefault() const
{
    Q_D(const DIpv6Setting);

    return d->m_neverDefault;
}

void DIpv6Setting::setMayFail(bool mayFail)
{
    Q_D(DIpv6Setting);

    d->m_mayFail = mayFail;
}

bool DIpv6Setting::mayFail() const
{
    Q_D(const DIpv6Setting);

    return d->m_mayFail;
}

void DIpv6Setting::setPrivacy(IPv6Privacy privacy)
{
    Q_D(DIpv6Setting);

    d->m_privacy = privacy;
}

DIpv6Setting::IPv6Privacy DIpv6Setting::privacy() const
{
    Q_D(const DIpv6Setting);

    return d->m_privacy;
}

void DIpv6Setting::setDadTimeout(qint32 timeout)
{
    Q_D(DIpv6Setting);

    d->m_dadTimeout = timeout;
}

qint32 DIpv6Setting::dadTimeout() const
{
    Q_D(const DIpv6Setting);

    return d->m_dadTimeout;
}

void DIpv6Setting::setDhcpTimeout(qint32 timeout)
{
    Q_D(DIpv6Setting);

    d->m_dhcpTimeout = timeout;
}

qint32 DIpv6Setting::dhcpTimeout() const
{
    Q_D(const DIpv6Setting);

    return d->m_dhcpTimeout;
}

void DIpv6Setting::setDhcpHostname(const QString &hostname)
{
    Q_D(DIpv6Setting);

    d->m_dhcpHostname = hostname;
}

QString DIpv6Setting::dhcpHostname() const
{
    Q_D(const DIpv6Setting);

    return d->m_dhcpHostname;
}

void DIpv6Setting::setDhcpDuid(const QString &duid)
{
    Q_D(DIpv6Setting);

    d->m_dhcpDuid = duid;
}

QString DIpv6Setting::dhcpDuid() const
{
    Q_D(const DIpv6Setting);

    return d->m_dhcpDuid;
}

void DIpv6Setting::setDnsOptions(const QStringList &options)
{
    Q_D(DIpv6Setting);

    d->m_dnsOptions = options;
}

QStringList DIpv6Setting::dnsOptions() const
{
    Q_D(const DIpv6Setting);

    return d->m_dnsOptions;
}

void DIpv6Setting::setAddressData(const QList<Config> &addressData)
{
    Q_D(DIpv6Setting);

    d->m_addressData = addressData;
}

QList<Config> DIpv6Setting::addressData() const
{
    Q_D(const DIpv6Setting);

    return d->m_addressData;
}

void DIpv6Setting::setAddressGenMode(IPv6AddressGenMode mode)
{
    Q_D(DIpv6Setting);

    d->m_addressGenMode = mode;
}

DIpv6Setting::IPv6AddressGenMode DIpv6Setting::addressGenMode() const
{
    Q_D(const DIpv6Setting);

    return d->m_addressGenMode;
}

void DIpv6Setting::setRouteData(const QList<Config> &routeData)
{
    Q_D(DIpv6Setting);

    d->m_routeData = routeData;
}

QList<Config> DIpv6Setting::routeData() const
{
    Q_D(const DIpv6Setting);

    return d->m_routeData;
}

void DIpv6Setting::setToken(const QString &token)
{
    Q_D(DIpv6Setting);

    d->m_token = token;
}

QString DIpv6Setting::token() const
{
    Q_D(const DIpv6Setting);

    return d->m_token;
}

void DIpv6Setting::setRouteTable(quint32 routeTable)
{
    Q_D(DIpv6Setting);

    d->m_routeTable = routeTable;
}

quint32 DIpv6Setting::routeTable() const
{
    Q_D(const DIpv6Setting);

    return d->m_routeTable;
}

void DIpv6Setting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_METHOD))) {
        const QString methodType = setting.value(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_METHOD)).toString();

        if (methodType.toLower() == QLatin1String(DTK_NM_SETTING_IP6_CONFIG_METHOD_DHCP)) {
            setMethod(NMConfigMethod::Ipv6Dhcp);
        } else if (methodType.toLower() == QLatin1String(DTK_NM_SETTING_IP6_CONFIG_METHOD_LINK_LOCAL)) {
            setMethod(NMConfigMethod::LinkLocal);
        } else if (methodType.toLower() == QLatin1String(DTK_NM_SETTING_IP6_CONFIG_METHOD_MANUAL)) {
            setMethod(NMConfigMethod::Manual);
        } else if (methodType.toLower() == QLatin1String(DTK_NM_SETTING_IP6_CONFIG_METHOD_IGNORE)) {
            setMethod(NMConfigMethod::Ipv6Ignored);
        } else if (methodType.toLower() == QLatin1String(DTK_NM_SETTING_IP6_CONFIG_METHOD_DISABLED)) {
            setMethod(NMConfigMethod::Disabled);
        } else {
            setMethod(NMConfigMethod::Automatic);
        }
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DNS))) {
        QList<QHostAddress> dbusDns;
        QList<QByteArray> temp;
        if (setting.value(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DNS)).canConvert<QDBusArgument>()) {
            auto dnsArg = setting.value(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DNS)).value<QDBusArgument>();
            temp = qdbus_cast<QList<QByteArray>>(dnsArg);
        } else {
            temp = setting.value(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DNS)).value<QList<QByteArray>>();
        }

        for (const QByteArray &utmp : std::as_const(temp)) {
            dbusDns << ipv6AddressToHostAddress(utmp);
        }

        setDns(dbusDns);
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DNS_SEARCH))) {
        setDnsSearch(setting.value(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DNS_SEARCH)).toStringList());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_ROUTE_METRIC))) {
        setRouteMetric(setting.value(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_ROUTE_METRIC)).toInt());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_IGNORE_AUTO_ROUTES))) {
        setIgnoreAutoRoutes(setting.value(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_IGNORE_AUTO_ROUTES)).toBool());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_IGNORE_AUTO_DNS))) {
        setIgnoreAutoDns(setting.value(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_IGNORE_AUTO_DNS)).toBool());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_NEVER_DEFAULT))) {
        setNeverDefault(setting.value(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_NEVER_DEFAULT)).toBool());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_MAY_FAIL))) {
        setMayFail(setting.value(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_MAY_FAIL)).toBool());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_IP6_PRIVACY))) {
        setPrivacy((IPv6Privacy)setting.value(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_IP6_PRIVACY)).toUInt());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DAD_TIMEOUT))) {
        setDadTimeout(setting.value(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DAD_TIMEOUT)).toInt());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DHCP_TIMEOUT))) {
        setDhcpTimeout(setting.value(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DHCP_TIMEOUT)).toInt());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_ADDRESS_GEN_MODE))) {
        setAddressGenMode(
            static_cast<IPv6AddressGenMode>(setting.value(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_ADDRESS_GEN_MODE)).toUInt()));
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DHCP_HOSTNAME))) {
        setDhcpHostname(setting.value(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DHCP_HOSTNAME)).toString());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DHCP_DUID))) {
        setDhcpDuid(setting.value(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DHCP_DUID)).toString());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_TOKEN))) {
        setToken(setting.value(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_TOKEN)).toString());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DNS_OPTIONS))) {
        setDnsOptions(setting.value(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DNS_OPTIONS)).toStringList());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_ROUTE_DATA))) {
        setRouteData(qdbus_cast<QList<Config>>(setting.value(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_ROUTE_DATA))));
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_ADDRESS_DATA))) {
        setAddressData(qdbus_cast<QList<Config>>(setting.value(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_ADDRESS_DATA))));
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_ROUTE_TABLE))) {
        setRouteTable(setting.value(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_ROUTE_TABLE)).toUInt());
    }
}

QVariantMap DIpv6Setting::toMap() const
{
    QVariantMap setting;

    if (method() == NMConfigMethod::Automatic) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_METHOD), QLatin1String(DTK_NM_SETTING_IP6_CONFIG_METHOD_AUTO));
    } else if (method() == NMConfigMethod::Ipv6Dhcp) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_METHOD), QLatin1String(DTK_NM_SETTING_IP6_CONFIG_METHOD_DHCP));
    } else if (method() == NMConfigMethod::LinkLocal) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_METHOD),
                       QLatin1String(DTK_NM_SETTING_IP6_CONFIG_METHOD_LINK_LOCAL));
    } else if (method() == NMConfigMethod::Manual) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_METHOD), QLatin1String(DTK_NM_SETTING_IP6_CONFIG_METHOD_MANUAL));
    } else if (method() == NMConfigMethod::Ipv6Ignored) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_METHOD), QLatin1String(DTK_NM_SETTING_IP6_CONFIG_METHOD_IGNORE));
    } else if (method() == NMConfigMethod::Disabled) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_METHOD), QLatin1String(DTK_NM_SETTING_IP6_CONFIG_METHOD_DISABLED));
    }

    if (!dns().isEmpty()) {
        QList<QByteArray> dbusDns;
        const QList<QHostAddress> dnsList = dns();
        for (const QHostAddress &dns : dnsList) {
            dbusDns << HostAddressToipv6Address(dns);
        }
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DNS), QVariant::fromValue(dbusDns));
    }

    if (!dnsSearch().isEmpty()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DNS_SEARCH), dnsSearch());
    }

    if (routeMetric() >= 0) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_ROUTE_METRIC), routeMetric());
    }

    if (ignoreAutoRoutes()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_IGNORE_AUTO_ROUTES), ignoreAutoRoutes());
    }

    if (ignoreAutoDns()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_IGNORE_AUTO_DNS), ignoreAutoDns());
    }

    if (neverDefault()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_NEVER_DEFAULT), neverDefault());
    }

    if (!mayFail()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_MAY_FAIL), mayFail());
    }

    if (privacy() != DIpv6Setting::IPv6Privacy::Unknown) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_IP6_PRIVACY), QVariant::fromValue(privacy()));
    }

    if (dadTimeout() >= 0) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DAD_TIMEOUT), dadTimeout());
    }

    setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_ADDRESS_GEN_MODE), QVariant::fromValue(addressGenMode()));

    if (dhcpTimeout() > 0) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DHCP_TIMEOUT), dhcpTimeout());
    }

    if (!dhcpHostname().isEmpty()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DHCP_HOSTNAME), dhcpHostname());
    }

    if (!dhcpDuid().isEmpty()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DHCP_DUID), dhcpDuid());
    }

    if (!token().isEmpty()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_TOKEN), token());
    }

    if (!dnsOptions().isEmpty()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_DNS_OPTIONS), dnsOptions());
    }

    if (!addressData().empty()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_ADDRESS_DATA), QVariant::fromValue(addressData()));
    }

    if (!routeData().empty()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_ROUTE_DATA), QVariant::fromValue(routeData()));
    }

    if (routeTable() > 0) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP6_CONFIG_ROUTE_TABLE), routeTable());
    }

    return setting;
}

DNETWORKMANAGER_END_NAMESPACE