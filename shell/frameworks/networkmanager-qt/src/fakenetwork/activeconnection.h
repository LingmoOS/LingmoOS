/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_FAKE_NETWORK_SETTINGS_ACTIVE_CONNECTION_H
#define NETWORKMANAGERQT_FAKE_NETWORK_SETTINGS_ACTIVE_CONNECTION_H

#include <QObject>

#include <QDBusObjectPath>

#include "../device.h"

#include "device.h"

class ActiveConnection : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.fakenetwork.Connection.Active")
public:
    explicit ActiveConnection(QObject *parent = nullptr);
    ~ActiveConnection() override;

    Q_PROPERTY(QDBusObjectPath Connection READ connection)
    Q_PROPERTY(bool Default READ default4)
    Q_PROPERTY(bool Default6 READ default6)
    Q_PROPERTY(QList<QDBusObjectPath> Devices READ devices)
    Q_PROPERTY(QDBusObjectPath Dhcp4Config READ dhcp4Config)
    Q_PROPERTY(QDBusObjectPath Dhcp6Config READ dhcp6Config)
    Q_PROPERTY(QDBusObjectPath Ip4Config READ ip4Config)
    Q_PROPERTY(QDBusObjectPath Ip6Config READ ip6Config)
    Q_PROPERTY(QDBusObjectPath Master READ master)
    Q_PROPERTY(QDBusObjectPath SpecificObject READ specificObject)
    Q_PROPERTY(uint State READ state)
    Q_PROPERTY(QString Id READ id)
    Q_PROPERTY(QString Uuid READ uuid)
    Q_PROPERTY(QString Type READ type)
    Q_PROPERTY(bool Vpn READ vpn)

    QDBusObjectPath connection() const;
    bool default4() const;
    bool default6() const;
    QList<QDBusObjectPath> devices() const;
    QDBusObjectPath dhcp4Config() const;
    QDBusObjectPath dhcp6Config() const;
    QDBusObjectPath ip4Config() const;
    QDBusObjectPath ip6Config() const;
    QDBusObjectPath master() const;
    QDBusObjectPath specificObject() const;
    uint state() const;
    QString id() const;
    QString uuid() const;
    QString type() const;
    bool vpn() const;

    /* Not part of DBus interface */
    void addDevice(const QDBusObjectPath &path);
    void removeDevice(const QDBusObjectPath &device);
    QString activeConnectionPath() const;
    void setActiveConnectionPath(const QString &path);
    void setConnection(const QDBusObjectPath &connection);
    void setDefault4(bool default4);
    void setDefault6(bool default6);
    void setDhcp4Config(const QDBusObjectPath &dhcp4Config);
    void setDhcp6Config(const QDBusObjectPath &dhcp6Config);
    void setIpv4Config(const QDBusObjectPath &ipv4Config);
    void setIpv6Config(const QDBusObjectPath &ipv6Config);
    void setMaster(const QDBusObjectPath &master);
    void setSpecificObject(const QDBusObjectPath &specificObject);
    void setState(uint state);
    void setId(const QString &id);
    void setUuid(const QString &uuid);
    void setType(const QString &type);

Q_SIGNALS:
    Q_SCRIPTABLE void PropertiesChanged(const QVariantMap &properties);
    Q_SCRIPTABLE void StateChanged(uint state, uint reason);

private:
    QDBusObjectPath m_connection;
    bool m_default4;
    bool m_default6;
    QList<QDBusObjectPath> m_devices;
    QDBusObjectPath m_dhcp4Config;
    QDBusObjectPath m_dhcp6Config;
    QDBusObjectPath m_ip4Config;
    QDBusObjectPath m_ip6Config;
    QDBusObjectPath m_master;
    QDBusObjectPath m_specificObject;
    uint m_state;
    QString m_id;
    QString m_uuid;
    QString m_type;
    bool m_vpn;

    /* Not part of DBus interface */
    QString m_activeConnectionPath;
};

#endif
