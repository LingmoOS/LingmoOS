// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ACTIVECONNECTIONSERVICE_H
#define ACTIVECONNECTIONSERVICE_H

#include <QDBusObjectPath>

class FakeActiveConncetionService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.FakeNetworkManager.Connection.Active")
public:
    explicit FakeActiveConncetionService(QObject *parent = nullptr)
        : QObject(parent){};

    virtual ~FakeActiveConncetionService() override = default;

    Q_PROPERTY(QList<QDBusObjectPath> Devices READ devices)
    Q_PROPERTY(bool Vpn READ vpn)
    Q_PROPERTY(QDBusObjectPath Connection READ connection)
    Q_PROPERTY(QDBusObjectPath Dhcp4Config READ DHCP4Config)
    Q_PROPERTY(QDBusObjectPath Dhcp6Config READ DHCP6Config)
    Q_PROPERTY(QDBusObjectPath Ip4Config READ IP4Config)
    Q_PROPERTY(QDBusObjectPath Ip6Config READ IP6Config)
    Q_PROPERTY(QDBusObjectPath SpecificObject READ specificObject)
    Q_PROPERTY(QString Id READ id)
    Q_PROPERTY(QString Uuid READ uuid)
    Q_PROPERTY(QString Type READ type)
    Q_PROPERTY(quint32 State READ state)

    QList<QDBusObjectPath> m_devices{QDBusObjectPath{"/org/freedesktop/NetworkManager/Devices/2"}};
    bool m_vpn{false};
    QDBusObjectPath m_connection{"/org/freedesktop/NetworkManager/Settings/4"};
    QDBusObjectPath m_DHCP4Config{"/org/freedesktop/NetworkManager/DHCP4Config/2"};
    QDBusObjectPath m_DHCP6Config{"/"};
    QDBusObjectPath m_IP4Config{"/org/freedesktop/NetworkManager/IP4Config/4"};
    QDBusObjectPath m_IP6Config{"/org/freedesktop/NetworkManager/IP6Config/4"};
    QDBusObjectPath m_specificObject{"/"};
    QString m_id{"有线连接"};
    QString m_type{"802-3-ethernet"};
    QString m_uuid{"eb7e3c30-d5de-4a21-a697-1c31777b4276"};
    quint32 m_state{2};

    QList<QDBusObjectPath> devices() const { return m_devices; }
    bool vpn() const { return m_vpn; }
    QDBusObjectPath connection() const { return m_connection; }
    QDBusObjectPath DHCP4Config() const { return m_DHCP4Config; }
    QDBusObjectPath DHCP6Config() const { return m_DHCP6Config; }
    QDBusObjectPath IP4Config() const { return m_IP4Config; }
    QDBusObjectPath IP6Config() const { return m_IP6Config; }
    QDBusObjectPath specificObject() const { return m_specificObject; }
    QString id() const { return m_id; }
    QString type() const { return m_type; }
    QString uuid() const { return m_uuid; }
    quint32 state() const { return m_state; }
};

#endif
