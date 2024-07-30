/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_IPV4_SETTING_H
#define NETWORKMANAGERQT_IPV4_SETTING_H

#include "ipconfig.h"
#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QHostAddress>
#include <QStringList>

#define NMQT_SETTING_IP4_CONFIG_SETTING_NAME NM_SETTING_IP4_CONFIG_SETTING_NAME
#define NMQT_SETTING_IP4_CONFIG_METHOD NM_SETTING_IP_CONFIG_METHOD
#define NMQT_SETTING_IP4_CONFIG_DNS NM_SETTING_IP_CONFIG_DNS
#define NMQT_SETTING_IP4_CONFIG_DNS_SEARCH NM_SETTING_IP_CONFIG_DNS_SEARCH
#define NMQT_SETTING_IP4_CONFIG_ADDRESSES NM_SETTING_IP_CONFIG_ADDRESSES
#define NMQT_SETTING_IP4_CONFIG_GATEWAY NM_SETTING_IP_CONFIG_GATEWAY
#define NMQT_SETTING_IP4_CONFIG_ROUTES NM_SETTING_IP_CONFIG_ROUTES
#define NMQT_SETTING_IP4_CONFIG_ROUTE_METRIC NM_SETTING_IP_CONFIG_ROUTE_METRIC
#define NMQT_SETTING_IP4_CONFIG_IGNORE_AUTO_ROUTES NM_SETTING_IP_CONFIG_IGNORE_AUTO_ROUTES
#define NMQT_SETTING_IP4_CONFIG_IGNORE_AUTO_DNS NM_SETTING_IP_CONFIG_IGNORE_AUTO_DNS
#define NMQT_SETTING_IP4_CONFIG_DHCP_HOSTNAME NM_SETTING_IP_CONFIG_DHCP_HOSTNAME
#define NMQT_SETTING_IP4_CONFIG_DHCP_SEND_HOSTNAME NM_SETTING_IP_CONFIG_DHCP_SEND_HOSTNAME
#define NMQT_SETTING_IP4_CONFIG_NEVER_DEFAULT NM_SETTING_IP_CONFIG_NEVER_DEFAULT
#define NMQT_SETTING_IP4_CONFIG_MAY_FAIL NM_SETTING_IP_CONFIG_MAY_FAIL
#define NMQT_SETTING_IP4_CONFIG_DHCP_CLIENT_ID NM_SETTING_IP4_CONFIG_DHCP_CLIENT_ID
#define NMQT_SETTING_IP4_CONFIG_METHOD_AUTO NM_SETTING_IP4_CONFIG_METHOD_AUTO
#define NMQT_SETTING_IP4_CONFIG_METHOD_LINK_LOCAL NM_SETTING_IP4_CONFIG_METHOD_LINK_LOCAL
#define NMQT_SETTING_IP4_CONFIG_METHOD_MANUAL NM_SETTING_IP4_CONFIG_METHOD_MANUAL
#define NMQT_SETTING_IP4_CONFIG_METHOD_SHARED NM_SETTING_IP4_CONFIG_METHOD_SHARED
#define NMQT_SETTING_IP4_CONFIG_METHOD_DISABLED NM_SETTING_IP4_CONFIG_METHOD_DISABLED
#define NMQT_SETTING_IP4_CONFIG_DAD_TIMEOUT NM_SETTING_IP_CONFIG_DAD_TIMEOUT
#define NMQT_SETTING_IP4_CONFIG_DHCP_FQDN NM_SETTING_IP4_CONFIG_DHCP_FQDN
#define NMQT_SETTING_IP4_CONFIG_DNS_OPTIONS NM_SETTING_IP_CONFIG_DNS_OPTIONS
#define NMQT_SETTING_IP4_CONFIG_DNS_PRIORITY NM_SETTING_IP_CONFIG_DNS_PRIORITY
#define NMQT_SETTING_IP4_CONFIG_GATEWAY NM_SETTING_IP_CONFIG_GATEWAY
#define NMQT_SETTING_IP4_CONFIG_ROUTE_DATA "route-data"
#define NMQT_SETTING_IP4_CONFIG_ADDRESS_DATA "address-data"

namespace NetworkManager
{
class Ipv4SettingPrivate;

/**
 * Represents ipv4 setting
 */
class NETWORKMANAGERQT_EXPORT Ipv4Setting : public Setting
{
public:
    typedef QSharedPointer<Ipv4Setting> Ptr;
    typedef QList<Ptr> List;
    enum ConfigMethod {
        Automatic,
        LinkLocal,
        Manual,
        Shared,
        Disabled,
    };

    Ipv4Setting();
    explicit Ipv4Setting(const Ptr &other);
    ~Ipv4Setting() override;

    QString name() const override;

    void setMethod(ConfigMethod method);
    ConfigMethod method() const;

    void setDns(const QList<QHostAddress> &dns);
    QList<QHostAddress> dns() const;

    void setDnsSearch(const QStringList &domains);
    QStringList dnsSearch() const;

    void setAddresses(const QList<NetworkManager::IpAddress> &ipv4addresses);
    QList<NetworkManager::IpAddress> addresses() const;

    void setRoutes(const QList<NetworkManager::IpRoute> &ipv4routes);
    QList<NetworkManager::IpRoute> routes() const;

    void setRouteMetric(int metric);
    int routeMetric() const;

    void setIgnoreAutoRoutes(bool ignore);
    bool ignoreAutoRoutes() const;

    void setIgnoreAutoDns(bool ignore);
    bool ignoreAutoDns() const;

    void setDhcpClientId(const QString &id);
    QString dhcpClientId() const;

    void setDhcpSendHostname(bool send);
    bool dhcpSendHostname() const;

    void setDhcpHostname(const QString &hostname);
    QString dhcpHostname() const;

    void setNeverDefault(bool neverDefault);
    bool neverDefault() const;

    void setMayFail(bool mayFail);
    bool mayFail() const;

    void setDadTimeout(qint32 timeout);
    qint32 dadTimeout() const;

    void setDhcpFqdn(const QString &fqdn);
    QString dhcpFqdn() const;

    void setDnsOptions(const QStringList &options);
    QStringList dnsOptions() const;

    void setDnsPriority(qint32 priority);
    qint32 dnsPriority() const;

    void setGateway(const QString &gateway);
    QString gateway() const;

    void setAddressData(const NMVariantMapList &addressData);
    NMVariantMapList addressData() const;

    void setRouteData(const NMVariantMapList &routeData);
    NMVariantMapList routeData() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    Ipv4SettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(Ipv4Setting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const Ipv4Setting &setting);

}

#endif // NETWORKMANAGERQT_IPV4_SETTING_H
