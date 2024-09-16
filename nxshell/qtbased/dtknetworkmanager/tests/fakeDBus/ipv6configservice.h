// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef IPV6CONFIGSERVICE_H
#define IPV6CONFIGSERVICE_H

#include "dnetworkmanagertypes.h"
#include <QList>
#include <QDBusObjectPath>
#include <QHostAddress>
#include <string>

DNETWORKMANAGER_USE_NAMESPACE

class FakeIPv6ConfigService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.FakeNetworkManager.IP6Config")
public:
    explicit FakeIPv6ConfigService(QObject *parent = nullptr);
    ~FakeIPv6ConfigService() override;

    Q_PROPERTY(QList<Config> AddressData READ addressData)
    Q_PROPERTY(QList<QByteArray> Nameservers READ nameservers)
    Q_PROPERTY(QString Gateway READ gateway)

    QList<Config> m_addressData{{{"address", QVariant::fromValue<QString>("::1")}}};
    QList<QByteArray> m_nameservers{{genIpv6Host()}};
    QString m_gateway{"fe80::1"};

    QList<Config> addressData() const { return m_addressData; };
    QList<QByteArray> nameservers() const { return m_nameservers; };
    QString gateway() const { return m_gateway; };

private:
    void registerService();
    void unregisterService();
    const QString Service{"com.deepin.FakeNetworkManager"};
    const QString Path{"/com/deepin/FakeNetworkManager/IP6Config/2"};
    static QByteArray genIpv6Host()
    {
        QByteArray ret;
        quint8 host[16]{32, 1, 72, 96, 72, 96, 0, 0, 0, 0, 0, 0, 0, 0, 136, 136};
        for (auto i : host)
            ret.append(i);
        return ret;
    }
};

#endif
