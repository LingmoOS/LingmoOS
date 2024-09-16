// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DHCP6CONFIGSERVICE_H
#define DHCP6CONFIGSERVICE_H

#include "dnetworkmanagertypes.h"
#include <QDBusObjectPath>

DNETWORKMANAGER_USE_NAMESPACE

class FakeDHCP6ConfigService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.FakeNetworkManager.DHCP6Config")
public:
    explicit FakeDHCP6ConfigService(QObject *parent = nullptr);
    ~FakeDHCP6ConfigService() override;

    Q_PROPERTY(Config Options READ options)

    Config m_options{{"ip_address", QVariant::fromValue<QString>("::1/128")}};

    Config options() const { return m_options; }

private:
    void registerService();
    void unregisterService();
    const QString Service{"com.deepin.FakeNetworkManager"};
    const QString Path{"/com/deepin/FakeNetworkManager/DHCP6Config/2"};
};

#endif
