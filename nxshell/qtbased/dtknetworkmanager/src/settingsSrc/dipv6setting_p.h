// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DIPV6SETTING_P_H
#define DIPV6SETTING_P_H

#include "dipv6setting.h"
#include <QStringList>

DNETWORKMANAGER_BEGIN_NAMESPACE

class DIpv6SettingPrivate
{
public:
    DIpv6SettingPrivate();

    bool m_ignoreAutoRoutes;
    bool m_ignoreAutoDns;
    bool m_neverDefault;
    bool m_mayFail;
    int m_routeMetric;
    NMConfigMethod m_method;
    quint32 m_routeTable;
    qint32 m_dadTimeout;
    qint32 m_dhcpTimeout;
    DIpv6Setting::IPv6Privacy m_privacy;
    DIpv6Setting::IPv6AddressGenMode m_addressGenMode;
    QString m_dhcpHostname;
    QString m_dhcpDuid;
    QString m_token;
    QString m_name;
    QStringList m_dnsOptions;
    QStringList m_dnsSearch;
    QList<Config> m_addressData;
    QList<Config> m_routeData;
    QList<QHostAddress> m_dns;
};

DNETWORKMANAGER_END_NAMESPACE

#endif
