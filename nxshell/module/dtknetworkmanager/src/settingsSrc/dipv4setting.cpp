// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dipv4setting_p.h"
#include <QDBusArgument>
#include <arpa/inet.h>

DNETWORKMANAGER_BEGIN_NAMESPACE

DIpv4SettingPrivate::DIpv4SettingPrivate()
    : m_ignoreAutoRoutes(false)
    , m_ignoreAutoDns(false)
    , m_neverDefault(false)
    , m_mayFail(true)
    , m_dhcpSendHostname(true)
    , m_routeMetric(-1)
    , m_dadTimeout(-1)
    , m_dnsPriority(0)
    , m_method(NMConfigMethod::Automatic)
    , m_name(DTK_NM_SETTING_IP4_CONFIG_SETTING_NAME)
{
}

DIpv4Setting::DIpv4Setting()
    : DNMSetting(DNMSetting::SettingType::Ipv4)
    , d_ptr(new DIpv4SettingPrivate())
{
}

DIpv4Setting::DIpv4Setting(const QSharedPointer<DIpv4Setting> &other)
    : DNMSetting(other)
    , d_ptr(new DIpv4SettingPrivate())
{
    setMethod(other->method());
    setDns(other->dns());
    setDnsSearch(other->dnsSearch());
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

QString DIpv4Setting::name() const
{
    Q_D(const DIpv4Setting);

    return d->m_name;
}

void DIpv4Setting::setMethod(NMConfigMethod type)
{
    Q_D(DIpv4Setting);

    d->m_method = type;
}

NMConfigMethod DIpv4Setting::method() const
{
    Q_D(const DIpv4Setting);

    return d->m_method;
}

void DIpv4Setting::setDns(const QList<QHostAddress> &dns)
{
    Q_D(DIpv4Setting);

    d->m_dns = dns;
}

QList<QHostAddress> DIpv4Setting::dns() const
{
    Q_D(const DIpv4Setting);

    return d->m_dns;
}

void DIpv4Setting::setDnsSearch(const QStringList &domains)
{
    Q_D(DIpv4Setting);

    d->m_dnsSearch = domains;
}

QStringList DIpv4Setting::dnsSearch() const
{
    Q_D(const DIpv4Setting);

    return d->m_dnsSearch;
}

void DIpv4Setting::setRouteMetric(int metric)
{
    Q_D(DIpv4Setting);

    d->m_routeMetric = metric;
}

int DIpv4Setting::routeMetric() const
{
    Q_D(const DIpv4Setting);

    return d->m_routeMetric;
}

void DIpv4Setting::setIgnoreAutoRoutes(bool ignore)
{
    Q_D(DIpv4Setting);

    d->m_ignoreAutoRoutes = ignore;
}

bool DIpv4Setting::ignoreAutoRoutes() const
{
    Q_D(const DIpv4Setting);

    return d->m_ignoreAutoRoutes;
}

void DIpv4Setting::setIgnoreAutoDns(bool ignore)
{
    Q_D(DIpv4Setting);

    d->m_ignoreAutoDns = ignore;
}

bool DIpv4Setting::ignoreAutoDns() const
{
    Q_D(const DIpv4Setting);

    return d->m_ignoreAutoDns;
}

void DIpv4Setting::setDhcpClientId(const QString &id)
{
    Q_D(DIpv4Setting);

    d->m_dhcpClientId = id;
}

QString DIpv4Setting::dhcpClientId() const
{
    Q_D(const DIpv4Setting);

    return d->m_dhcpClientId;
}

void DIpv4Setting::setDhcpSendHostname(bool send)
{
    Q_D(DIpv4Setting);

    d->m_dhcpSendHostname = send;
}

bool DIpv4Setting::dhcpSendHostname() const
{
    Q_D(const DIpv4Setting);

    return d->m_dhcpSendHostname;
}

void DIpv4Setting::setDhcpHostname(const QString &hostname)
{
    Q_D(DIpv4Setting);

    d->m_dhcpHostname = hostname;
}

QString DIpv4Setting::dhcpHostname() const
{
    Q_D(const DIpv4Setting);

    return d->m_dhcpHostname;
}

void DIpv4Setting::setNeverDefault(bool neverDefault)
{
    Q_D(DIpv4Setting);

    d->m_neverDefault = neverDefault;
}

bool DIpv4Setting::neverDefault() const
{
    Q_D(const DIpv4Setting);

    return d->m_neverDefault;
}

void DIpv4Setting::setMayFail(bool mayFail)
{
    Q_D(DIpv4Setting);

    d->m_mayFail = mayFail;
}

bool DIpv4Setting::mayFail() const
{
    Q_D(const DIpv4Setting);

    return d->m_mayFail;
}

void DIpv4Setting::setDadTimeout(qint32 timeout)
{
    Q_D(DIpv4Setting);

    d->m_dadTimeout = timeout;
}

qint32 DIpv4Setting::dadTimeout() const
{
    Q_D(const DIpv4Setting);

    return d->m_dadTimeout;
}

void DIpv4Setting::setDhcpFqdn(const QString &fqdn)
{
    Q_D(DIpv4Setting);

    d->m_dhcpFqdn = fqdn;
}

QString DIpv4Setting::dhcpFqdn() const
{
    Q_D(const DIpv4Setting);

    return d->m_dhcpFqdn;
}

void DIpv4Setting::setDnsOptions(const QStringList &options)
{
    Q_D(DIpv4Setting);

    d->m_dnsOptions = options;
}

QStringList DIpv4Setting::dnsOptions() const
{
    Q_D(const DIpv4Setting);

    return d->m_dnsOptions;
}

void DIpv4Setting::setDnsPriority(qint32 priority)
{
    Q_D(DIpv4Setting);

    d->m_dnsPriority = priority;
}

qint32 DIpv4Setting::dnsPriority() const
{
    Q_D(const DIpv4Setting);

    return d->m_dnsPriority;
}

void DIpv4Setting::setGateway(const QString &gateway)
{
    Q_D(DIpv4Setting);

    d->m_gateway = gateway;
}

QString DIpv4Setting::gateway() const
{
    Q_D(const DIpv4Setting);

    return d->m_gateway;
}

void DIpv4Setting::setAddressData(const QList<Config> &addressData)
{
    Q_D(DIpv4Setting);

    d->m_addressData = addressData;
}

QList<Config> DIpv4Setting::addressData() const
{
    Q_D(const DIpv4Setting);

    return d->m_addressData;
}

void DIpv4Setting::setRouteData(const QList<Config> &routeData)
{
    Q_D(DIpv4Setting);

    d->m_routeData = routeData;
}

QList<Config> DIpv4Setting::routeData() const
{
    Q_D(const DIpv4Setting);

    return d->m_routeData;
}

void DIpv4Setting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_METHOD))) {
        const QString methodType = setting.value(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_METHOD)).toString();

        if (methodType.toLower() == QLatin1String(DTK_NM_SETTING_IP4_CONFIG_METHOD_LINK_LOCAL)) {
            setMethod(NMConfigMethod::LinkLocal);
        } else if (methodType.toLower() == QLatin1String(DTK_NM_SETTING_IP4_CONFIG_METHOD_MANUAL)) {
            setMethod(NMConfigMethod::Manual);
        } else if (methodType.toLower() == QLatin1String(DTK_NM_SETTING_IP4_CONFIG_METHOD_SHARED)) {
            setMethod(NMConfigMethod::Ipv4Shared);
        } else if (methodType.toLower() == QLatin1String(DTK_NM_SETTING_IP4_CONFIG_METHOD_DISABLED)) {
            setMethod(NMConfigMethod::Disabled);
        } else {
            setMethod(NMConfigMethod::Automatic);
        }
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DNS))) {
        QList<QHostAddress> dbusDns;
        QList<uint> temp;
        if (setting.value(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DNS)).canConvert<QDBusArgument>()) {
            auto dnsArg = setting.value(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DNS)).value<QDBusArgument>();
            temp = qdbus_cast<QList<uint>>(dnsArg);
        } else {
            temp = setting.value(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DNS)).value<QList<uint>>();
        }

        for (const uint utmp : std::as_const(temp)) {
            QHostAddress tmpHost(ntohl(utmp));
            dbusDns << tmpHost;
        }

        setDns(dbusDns);
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DNS_SEARCH))) {
        setDnsSearch(setting.value(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DNS_SEARCH)).toStringList());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_ROUTE_METRIC))) {
        setRouteMetric(setting.value(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_ROUTE_METRIC)).toInt());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_IGNORE_AUTO_ROUTES))) {
        setIgnoreAutoRoutes(setting.value(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_IGNORE_AUTO_ROUTES)).toBool());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_IGNORE_AUTO_DNS))) {
        setIgnoreAutoDns(setting.value(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_IGNORE_AUTO_DNS)).toBool());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DHCP_CLIENT_ID))) {
        setDhcpClientId(setting.value(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DHCP_CLIENT_ID)).toString());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DHCP_SEND_HOSTNAME))) {
        setDhcpSendHostname(setting.value(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DHCP_SEND_HOSTNAME)).toBool());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DHCP_HOSTNAME))) {
        setDhcpHostname(setting.value(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DHCP_HOSTNAME)).toString());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_NEVER_DEFAULT))) {
        setNeverDefault(setting.value(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_NEVER_DEFAULT)).toBool());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_MAY_FAIL))) {
        setMayFail(setting.value(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_MAY_FAIL)).toBool());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DAD_TIMEOUT))) {
        setDadTimeout(setting.value(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DAD_TIMEOUT)).toInt());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DHCP_FQDN))) {
        setDhcpFqdn(setting.value(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DHCP_FQDN)).toString());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DNS_OPTIONS))) {
        setDnsOptions(setting.value(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DNS_OPTIONS)).toStringList());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DNS_PRIORITY))) {
        setDnsPriority(setting.value(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DNS_PRIORITY)).toInt());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_GATEWAY))) {
        setGateway(setting.value(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_GATEWAY)).toString());
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_ROUTE_DATA))) {
        setRouteData(qdbus_cast<QList<Config>>(setting.value(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_ROUTE_DATA))));
    }

    if (setting.contains(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_ADDRESS_DATA))) {
        setAddressData(qdbus_cast<QList<Config>>(setting.value(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_ADDRESS_DATA))));
    }
}

QVariantMap DIpv4Setting::toMap() const
{
    QVariantMap setting;

    if (method() == NMConfigMethod::Automatic) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_METHOD), QLatin1String(DTK_NM_SETTING_IP4_CONFIG_METHOD_AUTO));
    } else if (method() == NMConfigMethod::LinkLocal) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_METHOD),
                       QLatin1String(DTK_NM_SETTING_IP4_CONFIG_METHOD_LINK_LOCAL));
    } else if (method() == NMConfigMethod::Manual) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_METHOD), QLatin1String(DTK_NM_SETTING_IP4_CONFIG_METHOD_MANUAL));
    } else if (method() == NMConfigMethod::Ipv4Shared) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_METHOD), QLatin1String(DTK_NM_SETTING_IP4_CONFIG_METHOD_SHARED));
    } else if (method() == NMConfigMethod::Disabled) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_METHOD), QLatin1String(DTK_NM_SETTING_IP4_CONFIG_METHOD_DISABLED));
    }

    if (!dns().isEmpty()) {
        QList<uint> dbusDns;
        const QList<QHostAddress> dnsList = dns();
        for (const QHostAddress &dns : dnsList) {
            dbusDns << htonl(dns.toIPv4Address());
        }

        setting.insert(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DNS), QVariant::fromValue(dbusDns));
    }

    if (!dnsSearch().isEmpty()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DNS_SEARCH), dnsSearch());
    }

    if (routeMetric() >= 0) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_ROUTE_METRIC), routeMetric());
    }

    if (ignoreAutoRoutes()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_IGNORE_AUTO_ROUTES), ignoreAutoRoutes());
    }

    if (ignoreAutoDns()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_IGNORE_AUTO_DNS), ignoreAutoDns());
    }

    if (!dhcpClientId().isEmpty()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DHCP_CLIENT_ID), dhcpClientId());
    }

    if (!dhcpSendHostname()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DHCP_SEND_HOSTNAME), dhcpSendHostname());
    }

    if (!dhcpHostname().isEmpty()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DHCP_HOSTNAME), dhcpHostname());
    }

    if (neverDefault()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_NEVER_DEFAULT), neverDefault());
    }

    if (!mayFail()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_MAY_FAIL), mayFail());
    }

    if (dadTimeout() >= 0) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DAD_TIMEOUT), dadTimeout());
    }

    if (!dhcpFqdn().isEmpty()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DHCP_FQDN), dhcpFqdn());
    }

    if (!dnsOptions().isEmpty()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DNS_OPTIONS), dnsOptions());
    }

    if (dnsPriority() != 0) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_DNS_PRIORITY), dnsPriority());
    }

    if (!gateway().isEmpty()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_GATEWAY), gateway());
    }

    if (!addressData().empty()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_ADDRESS_DATA), QVariant::fromValue(addressData()));
    }

    if (!routeData().empty()) {
        setting.insert(QLatin1String(DTK_NM_SETTING_IP4_CONFIG_ROUTE_DATA), QVariant::fromValue(routeData()));
    }

    return setting;
}

DNETWORKMANAGER_END_NAMESPACE