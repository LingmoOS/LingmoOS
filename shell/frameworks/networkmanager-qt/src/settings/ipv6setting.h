/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_IPV6_SETTING_H
#define NETWORKMANAGERQT_IPV6_SETTING_H

#include "ipconfig.h"
#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QHostAddress>
#include <QStringList>

#define NMQT_SETTING_IP6_CONFIG_SETTING_NAME NM_SETTING_IP6_CONFIG_SETTING_NAME
#define NMQT_SETTING_IP6_CONFIG_METHOD NM_SETTING_IP_CONFIG_METHOD
#define NMQT_SETTING_IP6_CONFIG_DNS NM_SETTING_IP_CONFIG_DNS
#define NMQT_SETTING_IP6_CONFIG_DNS_SEARCH NM_SETTING_IP_CONFIG_DNS_SEARCH
#define NMQT_SETTING_IP6_CONFIG_ADDRESSES NM_SETTING_IP_CONFIG_ADDRESSES
#define NMQT_SETTING_IP6_CONFIG_GATEWAY NM_SETTING_IP_CONFIG_GATEWAY
#define NMQT_SETTING_IP6_CONFIG_ROUTES NM_SETTING_IP_CONFIG_ROUTES
#define NMQT_SETTING_IP6_CONFIG_ROUTE_METRIC NM_SETTING_IP_CONFIG_ROUTE_METRIC
#define NMQT_SETTING_IP6_CONFIG_IGNORE_AUTO_ROUTES NM_SETTING_IP_CONFIG_IGNORE_AUTO_ROUTES
#define NMQT_SETTING_IP6_CONFIG_IGNORE_AUTO_DNS NM_SETTING_IP_CONFIG_IGNORE_AUTO_DNS
#define NMQT_SETTING_IP6_CONFIG_DHCP_HOSTNAME NM_SETTING_IP_CONFIG_DHCP_HOSTNAME
#define NMQT_SETTING_IP6_CONFIG_DHCP_SEND_HOSTNAME NM_SETTING_IP_CONFIG_DHCP_SEND_HOSTNAME
#define NMQT_SETTING_IP6_CONFIG_NEVER_DEFAULT NM_SETTING_IP_CONFIG_NEVER_DEFAULT
#define NMQT_SETTING_IP6_CONFIG_MAY_FAIL NM_SETTING_IP_CONFIG_MAY_FAIL
#define NMQT_SETTING_IP6_CONFIG_DHCP_CLIENT_ID NM_SETTING_IP6_CONFIG_DHCP_CLIENT_ID
#define NMQT_SETTING_IP6_CONFIG_IP6_PRIVACY NM_SETTING_IP6_CONFIG_IP6_PRIVACY
#define NMQT_SETTING_IP6_CONFIG_METHOD_IGNORE NM_SETTING_IP6_CONFIG_METHOD_IGNORE
#define NMQT_SETTING_IP6_CONFIG_METHOD_AUTO NM_SETTING_IP6_CONFIG_METHOD_AUTO
#define NMQT_SETTING_IP6_CONFIG_METHOD_DHCP NM_SETTING_IP6_CONFIG_METHOD_DHCP
#define NMQT_SETTING_IP6_CONFIG_METHOD_LINK_LOCAL NM_SETTING_IP6_CONFIG_METHOD_LINK_LOCAL
#define NMQT_SETTING_IP6_CONFIG_METHOD_MANUAL NM_SETTING_IP6_CONFIG_METHOD_MANUAL
#define NMQT_SETTING_IP6_CONFIG_METHOD_SHARED NM_SETTING_IP6_CONFIG_METHOD_SHARED
#if NM_CHECK_VERSION(1, 20, 0)
#define NMQT_SETTING_IP6_CONFIG_METHOD_DISABLED NM_SETTING_IP6_CONFIG_METHOD_DISABLED
#else
#define NMQT_SETTING_IP6_CONFIG_METHOD_DISABLED "disabled"
#endif
#define NMQT_SETTING_IP6_CONFIG_DAD_TIMEOUT NM_SETTING_IP_CONFIG_DAD_TIMEOUT
#define NMQT_SETTING_IP6_CONFIG_DHCP_TIMEOUT NM_SETTING_IP_CONFIG_DHCP_TIMEOUT
#define NMQT_SETTING_IP6_CONFIG_DHCP_HOSTNAME NM_SETTING_IP_CONFIG_DHCP_HOSTNAME
#define NMQT_SETTING_IP6_CONFIG_ADDRESS_GEN_MODE NM_SETTING_IP6_CONFIG_ADDR_GEN_MODE
#if NM_CHECK_VERSION(1, 12, 0)
#define NMQT_SETTING_IP6_CONFIG_DHCP_DUID NM_SETTING_IP6_CONFIG_DHCP_DUID
#else
#define NMQT_SETTING_IP6_CONFIG_DHCP_DUID "dhcp-duid"
#endif
#define NMQT_SETTING_IP6_CONFIG_TOKEN NM_SETTING_IP6_CONFIG_TOKEN
#define NMQT_SETTING_IP6_CONFIG_DNS_OPTIONS NM_SETTING_IP_CONFIG_DNS_OPTIONS
#define NMQT_SETTING_IP6_CONFIG_ROUTE_DATA "route-data"
#define NMQT_SETTING_IP6_CONFIG_ADDRESS_DATA "address-data"
#if NM_CHECK_VERSION(1, 10, 0)
#define NMQT_SETTING_IP6_CONFIG_ROUTE_TABLE NM_SETTING_IP_CONFIG_ROUTE_TABLE
#else
#define NMQT_SETTING_IP6_CONFIG_ROUTE_TABLE "route-table"
#endif

namespace NetworkManager
{
class Ipv6SettingPrivate;

/**
 * Represents ipv6 setting
 */
class NETWORKMANAGERQT_EXPORT Ipv6Setting : public Setting
{
public:
    typedef QSharedPointer<Ipv6Setting> Ptr;
    typedef QList<Ptr> List;
    enum ConfigMethod {
        Automatic,
        Dhcp,
        LinkLocal,
        Manual,
        Ignored,
        ConfigDisabled // FIXME KF6: fix enum naming (possibly move to enum class)
    };
    enum IPv6Privacy {
        Unknown = -1,
        Disabled,
        PreferPublic,
        PreferTemporary,
    };
    enum IPv6AddressGenMode {
        Eui64,
        StablePrivacy,
    };

    Ipv6Setting();
    explicit Ipv6Setting(const Ptr &other);
    ~Ipv6Setting() override;

    QString name() const override;

    void setMethod(ConfigMethod method);
    ConfigMethod method() const;

    void setDns(const QList<QHostAddress> &dns);
    QList<QHostAddress> dns() const;

    void setDnsSearch(const QStringList &domains);
    QStringList dnsSearch() const;

    void setAddresses(const QList<NetworkManager::IpAddress> ipv6addresses);
    QList<NetworkManager::IpAddress> addresses() const;

    void setRoutes(const QList<NetworkManager::IpRoute> ipv6routes);
    QList<NetworkManager::IpRoute> routes() const;

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

    void setAddressData(const NMVariantMapList &addressData);
    NMVariantMapList addressData() const;

    void setRouteData(const NMVariantMapList &routeData);
    NMVariantMapList routeData() const;

    void setRouteTable(quint32 routeTable);
    quint32 routeTable() const;

protected:
    Ipv6SettingPrivate *const d_ptr;

private:
    Q_DECLARE_PRIVATE(Ipv6Setting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const Ipv6Setting &setting);

}

#endif // NETWORKMANAGERQT_IPV6_SETTING_H
