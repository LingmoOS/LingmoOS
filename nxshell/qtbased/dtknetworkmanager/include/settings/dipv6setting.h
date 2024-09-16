// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DIPV6SETTING_H
#define DIPV6SETTING_H

#include "dnmsetting.h"
#include "dnetworkmanagertypes.h"
#include <QStringList>
#include <QHostAddress>

#define DTK_NM_SETTING_IP6_CONFIG_SETTING_NAME NM_SETTING_IP6_CONFIG_SETTING_NAME
#define DTK_NM_SETTING_IP6_CONFIG_METHOD NM_SETTING_IP_CONFIG_METHOD
#define DTK_NM_SETTING_IP6_CONFIG_DNS NM_SETTING_IP_CONFIG_DNS
#define DTK_NM_SETTING_IP6_CONFIG_DNS_SEARCH NM_SETTING_IP_CONFIG_DNS_SEARCH
#define DTK_NM_SETTING_IP6_CONFIG_ADDRESSES NM_SETTING_IP_CONFIG_ADDRESSES
#define DTK_NM_SETTING_IP6_CONFIG_GATEWAY NM_SETTING_IP_CONFIG_GATEWAY
#define DTK_NM_SETTING_IP6_CONFIG_ROUTES NM_SETTING_IP_CONFIG_ROUTES
#define DTK_NM_SETTING_IP6_CONFIG_ROUTE_METRIC NM_SETTING_IP_CONFIG_ROUTE_METRIC
#define DTK_NM_SETTING_IP6_CONFIG_IGNORE_AUTO_ROUTES NM_SETTING_IP_CONFIG_IGNORE_AUTO_ROUTES
#define DTK_NM_SETTING_IP6_CONFIG_IGNORE_AUTO_DNS NM_SETTING_IP_CONFIG_IGNORE_AUTO_DNS
#define DTK_NM_SETTING_IP6_CONFIG_DHCP_HOSTNAME NM_SETTING_IP_CONFIG_DHCP_HOSTNAME
#define DTK_NM_SETTING_IP6_CONFIG_DHCP_SEND_HOSTNAME NM_SETTING_IP_CONFIG_DHCP_SEND_HOSTNAME
#define DTK_NM_SETTING_IP6_CONFIG_NEVER_DEFAULT NM_SETTING_IP_CONFIG_NEVER_DEFAULT
#define DTK_NM_SETTING_IP6_CONFIG_MAY_FAIL NM_SETTING_IP_CONFIG_MAY_FAIL
#define DTK_NM_SETTING_IP6_CONFIG_DHCP_CLIENT_ID NM_SETTING_IP6_CONFIG_DHCP_CLIENT_ID
#define DTK_NM_SETTING_IP6_CONFIG_IP6_PRIVACY NM_SETTING_IP6_CONFIG_IP6_PRIVACY
#define DTK_NM_SETTING_IP6_CONFIG_METHOD_IGNORE NM_SETTING_IP6_CONFIG_METHOD_IGNORE
#define DTK_NM_SETTING_IP6_CONFIG_METHOD_AUTO NM_SETTING_IP6_CONFIG_METHOD_AUTO
#define DTK_NM_SETTING_IP6_CONFIG_METHOD_DHCP NM_SETTING_IP6_CONFIG_METHOD_DHCP
#define DTK_NM_SETTING_IP6_CONFIG_METHOD_LINK_LOCAL NM_SETTING_IP6_CONFIG_METHOD_LINK_LOCAL
#define DTK_NM_SETTING_IP6_CONFIG_METHOD_MANUAL NM_SETTING_IP6_CONFIG_METHOD_MANUAL
#define DTK_NM_SETTING_IP6_CONFIG_METHOD_SHARED NM_SETTING_IP6_CONFIG_METHOD_SHARED
#if NM_CHECK_VERSION(1, 20, 0)
#define DTK_NM_SETTING_IP6_CONFIG_METHOD_DISABLED NM_SETTING_IP6_CONFIG_METHOD_DISABLED
#else
#define DTK_NM_SETTING_IP6_CONFIG_METHOD_DISABLED "disabled"
#endif
#define DTK_NM_SETTING_IP6_CONFIG_DAD_TIMEOUT NM_SETTING_IP_CONFIG_DAD_TIMEOUT
#define DTK_NM_SETTING_IP6_CONFIG_DHCP_TIMEOUT NM_SETTING_IP_CONFIG_DHCP_TIMEOUT
#define DTK_NM_SETTING_IP6_CONFIG_DHCP_HOSTNAME NM_SETTING_IP_CONFIG_DHCP_HOSTNAME
#define DTK_NM_SETTING_IP6_CONFIG_ADDRESS_GEN_MODE NM_SETTING_IP6_CONFIG_ADDR_GEN_MODE
#define DTK_NM_SETTING_IP6_CONFIG_DHCP_DUID NM_SETTING_IP6_CONFIG_DHCP_DUID
#define DTK_NM_SETTING_IP6_CONFIG_TOKEN NM_SETTING_IP6_CONFIG_TOKEN
#define DTK_NM_SETTING_IP6_CONFIG_DNS_OPTIONS NM_SETTING_IP_CONFIG_DNS_OPTIONS
#define DTK_NM_SETTING_IP6_CONFIG_ROUTE_TABLE NM_SETTING_IP_CONFIG_ROUTE_TABLE
#define DTK_NM_SETTING_IP6_CONFIG_ROUTE_DATA "route-data"
#define DTK_NM_SETTING_IP6_CONFIG_ADDRESS_DATA "address-data"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DIpv6SettingPrivate;

class DIpv6Setting : public DNMSetting
{
public:
    enum class IPv6Privacy {
        Unknown = -1,
        Disabled,
        PreferPublic,
        PreferTemporary,
    };
    enum class IPv6AddressGenMode {
        Eui64 = 0,
        StablePrivacy,
    };

    DIpv6Setting();
    explicit DIpv6Setting(const QSharedPointer<DIpv6Setting> &other);
    ~DIpv6Setting() override = default;

    QString name() const override;

    void setMethod(NMConfigMethod method);
    NMConfigMethod method() const;

    void setDns(const QList<QHostAddress> &dns);
    QList<QHostAddress> dns() const;

    void setDnsSearch(const QStringList &domains);
    QStringList dnsSearch() const;

    void setRouteMetric(int metric);
    int routeMetric() const;

    void setIgnoreAutoRoutes(bool ignore);
    bool ignoreAutoRoutes() const;

    void setIgnoreAutoDns(bool ignore);
    bool ignoreAutoDns() const;

    void setNeverDefault(bool neverDefault);
    bool neverDefault() const;

    void setMayFail(bool mayFail);
    bool mayFail() const;

    void setPrivacy(IPv6Privacy privacy);
    IPv6Privacy privacy() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

    void setDadTimeout(qint32 timeout);
    qint32 dadTimeout() const;

    void setAddressGenMode(IPv6AddressGenMode mode);
    IPv6AddressGenMode addressGenMode() const;

    void setDhcpTimeout(qint32 timeout);
    qint32 dhcpTimeout() const;

    void setDhcpHostname(const QString &hostname);
    QString dhcpHostname() const;

    void setDhcpDuid(const QString &duid);
    QString dhcpDuid() const;

    void setToken(const QString &token);
    QString token() const;

    void setDnsOptions(const QStringList &options);
    QStringList dnsOptions() const;

    void setAddressData(const QList<Config> &addressData);
    QList<Config> addressData() const;

    void setRouteData(const QList<Config> &routeData);
    QList<Config> routeData() const;

    void setRouteTable(quint32 routeTable);
    quint32 routeTable() const;

protected:
    QScopedPointer<DIpv6SettingPrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(DIpv6Setting)
};

DNETWORKMANAGER_END_NAMESPACE

Q_DECLARE_METATYPE(DTK_DNETWORKMANAGER_NAMESPACE::DIpv6Setting::IPv6Privacy)
Q_DECLARE_METATYPE(DTK_DNETWORKMANAGER_NAMESPACE::DIpv6Setting::IPv6AddressGenMode)

#endif
