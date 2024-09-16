// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ACTIVEVPNCONNECTION_H
#define ACTIVEVPNCONNECTION_H

#include <QDBusObjectPath>
#include "activeconnectionservice.h"

class FakeActiveVpnConncetionService : public FakeActiveConncetionService
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.FakeNetworkManager.VPN.Connection")
public:
    explicit FakeActiveVpnConncetionService(QObject *parent = nullptr);
    virtual ~FakeActiveVpnConncetionService() override;

    Q_PROPERTY(quint32 VpnState READ vpnState)
    Q_PROPERTY(QString Banner READ banner)

    quint32 m_vpnState{5};
    QString m_banner{"welcome"};

    quint32 vpnState() const { return m_vpnState; }
    QString banner() const { return m_banner; }

private:
    void registerService();
    void unregisterService();
    const QString Service{"com.deepin.FakeNetworkManager"};
    const QString Path{"/com/deepin/FakeNetworkManager/ActiveConnection/2"};
};

#endif
