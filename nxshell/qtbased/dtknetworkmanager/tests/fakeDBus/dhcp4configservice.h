// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DHCP4CONFIGSERVICE_H
#define DHCP4CONFIGSERVICE_H

#include "dnetworkmanagertypes.h"
#include <QDBusObjectPath>

DNETWORKMANAGER_USE_NAMESPACE

class FakeDHCP4ConfigService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.FakeNetworkManager.DHCP4Config")
public:
    explicit FakeDHCP4ConfigService(QObject *parent = nullptr);
    ~FakeDHCP4ConfigService() override;

    Q_PROPERTY(Config Options READ options)

    Config m_options{{"broadcast_address", QVariant::fromValue<QString>("1.1.1.1")}};

    Config options() const { return m_options; }

private:
    void registerService();
    void unregisterService();
    const QString Service{"com.deepin.FakeNetworkManager"};
    const QString Path{"/com/deepin/FakeNetworkManager/DHCP4Config/2"};
};

#endif
