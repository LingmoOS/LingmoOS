// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DIPV4SETTING_H
#define DIPV4SETTING_H

#include "dnmsetting.h"
#include "dnetworkmanagertypes.h"
#include <QStringList>
#include <QHostAddress>

#define DTK_NM_SETTING_IP4_CONFIG_SETTING_NAME NM_SETTING_IP4_CONFIG_SETTING_NAME
#define DTK_NM_SETTING_IP4_CONFIG_METHOD NM_SETTING_IP_CONFIG_METHOD
#define DTK_NM_SETTING_IP4_CONFIG_DNS NM_SETTING_IP_CONFIG_DNS
#define DTK_NM_SETTING_IP4_CONFIG_DNS_SEARCH NM_SETTING_IP_CONFIG_DNS_SEARCH
#define DTK_NM_SETTING_IP4_CONFIG_ADDRESSES NM_SETTING_IP_CONFIG_ADDRESSES
#define DTK_NM_SETTING_IP4_CONFIG_GATEWAY NM_SETTING_IP_CONFIG_GATEWAY
#define DTK_NM_SETTING_IP4_CONFIG_ROUTES NM_SETTING_IP_CONFIG_ROUTES
#define DTK_NM_SETTING_IP4_CONFIG_ROUTE_METRIC NM_SETTING_IP_CONFIG_ROUTE_METRIC
#define DTK_NM_SETTING_IP4_CONFIG_IGNORE_AUTO_ROUTES NM_SETTING_IP_CONFIG_IGNORE_AUTO_ROUTES
#define DTK_NM_SETTING_IP4_CONFIG_IGNORE_AUTO_DNS NM_SETTING_IP_CONFIG_IGNORE_AUTO_DNS
#define DTK_NM_SETTING_IP4_CONFIG_DHCP_HOSTNAME NM_SETTING_IP_CONFIG_DHCP_HOSTNAME
#define DTK_NM_SETTING_IP4_CONFIG_DHCP_SEND_HOSTNAME NM_SETTING_IP_CONFIG_DHCP_SEND_HOSTNAME
#define DTK_NM_SETTING_IP4_CONFIG_NEVER_DEFAULT NM_SETTING_IP_CONFIG_NEVER_DEFAULT
#define DTK_NM_SETTING_IP4_CONFIG_MAY_FAIL NM_SETTING_IP_CONFIG_MAY_FAIL
#define DTK_NM_SETTING_IP4_CONFIG_DHCP_CLIENT_ID NM_SETTING_IP4_CONFIG_DHCP_CLIENT_ID
#define DTK_NM_SETTING_IP4_CONFIG_METHOD_AUTO NM_SETTING_IP4_CONFIG_METHOD_AUTO
#define DTK_NM_SETTING_IP4_CONFIG_METHOD_LINK_LOCAL NM_SETTING_IP4_CONFIG_METHOD_LINK_LOCAL
#define DTK_NM_SETTING_IP4_CONFIG_METHOD_MANUAL NM_SETTING_IP4_CONFIG_METHOD_MANUAL
#define DTK_NM_SETTING_IP4_CONFIG_METHOD_SHARED NM_SETTING_IP4_CONFIG_METHOD_SHARED
#define DTK_NM_SETTING_IP4_CONFIG_METHOD_DISABLED NM_SETTING_IP4_CONFIG_METHOD_DISABLED
#define DTK_NM_SETTING_IP4_CONFIG_DAD_TIMEOUT NM_SETTING_IP_CONFIG_DAD_TIMEOUT
#define DTK_NM_SETTING_IP4_CONFIG_DHCP_FQDN NM_SETTING_IP4_CONFIG_DHCP_FQDN
#define DTK_NM_SETTING_IP4_CONFIG_DNS_OPTIONS NM_SETTING_IP_CONFIG_DNS_OPTIONS
#define DTK_NM_SETTING_IP4_CONFIG_DNS_PRIORITY NM_SETTING_IP_CONFIG_DNS_PRIORITY
#define DTK_NM_SETTING_IP4_CONFIG_GATEWAY NM_SETTING_IP_CONFIG_GATEWAY
#define DTK_NM_SETTING_IP4_CONFIG_ROUTE_DATA "route-data"
#define DTK_NM_SETTING_IP4_CONFIG_ADDRESS_DATA "address-data"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DIpv4SettingPrivate;

class DIpv4Setting : public DNMSetting
{
public:
    DIpv4Setting();
    explicit DIpv4Setting(const QSharedPointer<DIpv4Setting> &other);
    ~DIpv4Setting() override = default;
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

    void setAddressData(const QList<Config> &addressData);
    QList<Config> addressData() const;

    void setRouteData(const QList<Config> &routeData);
    QList<Config> routeData() const;

    void fromMap(const Config &setting) override;

    Config toMap() const override;

protected:
    QScopedPointer<DIpv4SettingPrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(DIpv4Setting)
};

DNETWORKMANAGER_END_NAMESPACE

#endif