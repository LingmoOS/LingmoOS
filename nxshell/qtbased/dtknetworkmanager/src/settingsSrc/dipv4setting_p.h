// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DIPV4SETTING_P_H
#define DIPV4SETTING_P_H

#include "dipv4setting.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DIpv4SettingPrivate
{
public:
    DIpv4SettingPrivate();

    bool m_ignoreAutoRoutes;
    bool m_ignoreAutoDns;
    bool m_neverDefault;
    bool m_mayFail;
    bool m_dhcpSendHostname;
    int m_routeMetric;
    qint32 m_dadTimeout;
    qint32 m_dnsPriority;
    NMConfigMethod m_method;
    QString m_name;
    QString m_dhcpClientId;
    QString m_dhcpHostname;
    QString m_dhcpFqdn;
    QString m_gateway;
    QList<Config> m_addressData;
    QList<Config> m_routeData;
    QList<QHostAddress> m_dns;
    QStringList m_dnsSearch;
    QStringList m_dnsOptions;
};

DNETWORKMANAGER_END_NAMESPACE

#endif