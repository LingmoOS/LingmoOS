// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef IPV4CONFIGSERVICE_H
#define IPV4CONFIGSERVICE_H

#include "dnetworkmanagertypes.h"
#include <QList>
#include <QDBusObjectPath>

DNETWORKMANAGER_USE_NAMESPACE

class FakeIPv4ConfigService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.FakeNetworkManager.IP4Config")
public:
    explicit FakeIPv4ConfigService(QObject *parent = nullptr);
    ~FakeIPv4ConfigService() override;

    Q_PROPERTY(QList<Config> AddressData READ addressData)
    Q_PROPERTY(QList<Config> NameserverData READ nameserverData)
    Q_PROPERTY(QString Gateway READ gateway)

    QList<Config> m_addressData{{{"address", QVariant::fromValue<QString>("192.168.0.100")}}};
    QList<Config> m_nameserverData{{{"address", QVariant::fromValue<QString>("1.1.1.1")}}};
    QString m_gateway{"192.168.0.1"};

    QList<Config> addressData() const { return m_addressData; };
    QList<Config> nameserverData() const { return m_nameserverData; };
    QString gateway() const { return m_gateway; };

private:
    void registerService();
    void unregisterService();
    const QString Service{"com.deepin.FakeNetworkManager"};
    const QString Path{"/com/deepin/FakeNetworkManager/IP4Config/2"};
};

#endif
