// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DEVICESERVICE_H
#define DEVICESERVICE_H

#include <QDBusObjectPath>

class FakeDeviceService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.FakeNetworkManager.Device")
public:
    explicit FakeDeviceService(QObject *parent = nullptr);
    virtual ~FakeDeviceService() override;

    Q_PROPERTY(QList<QDBusObjectPath> AvailableConnections READ availableConnections)
    Q_PROPERTY(bool Autoconnect READ autoconnect WRITE setAutoconnect)
    Q_PROPERTY(bool Managed READ managed WRITE setManaged)
    Q_PROPERTY(QDBusObjectPath ActiveConnection READ activeConnection)
    Q_PROPERTY(QDBusObjectPath Dhcp4Config READ DHCP4Config)
    Q_PROPERTY(QDBusObjectPath Dhcp6Config READ DHCP6Config)
    Q_PROPERTY(QDBusObjectPath Ip4Config READ IPv4Config)
    Q_PROPERTY(QDBusObjectPath Ip6Config READ IPv6Config)
    Q_PROPERTY(QString Driver READ driver)
    Q_PROPERTY(QString Interface READ interface)
    Q_PROPERTY(QString Udi READ udi)
    Q_PROPERTY(quint32 DeviceType READ deviceType)
    Q_PROPERTY(quint32 InterfaceFlags READ interfaceFlags)
    Q_PROPERTY(quint32 State READ state)

    QList<QDBusObjectPath> m_availableConnections{QDBusObjectPath{"/org/freedesktop/NetworkManager/Settings/2"}};
    bool m_autoconnect{true};
    bool m_managed{true};
    QDBusObjectPath m_activeConnection{"/org/freedesktop/NetworkManager/ActiveConnection/1"};
    QDBusObjectPath m_DHCP4Config{"/org/freedesktop/NetworkManager/DHCP4Config/1"};
    QDBusObjectPath m_DHCP6Config{"/org/freedesktop/NetworkManager/DHCP6Config/1"};
    QDBusObjectPath m_IPv4Config{"/org/freedesktop/NetworkManager/IP4Config/1"};
    QDBusObjectPath m_IPv6Config{"/org/freedesktop/NetworkManager/IP6Config/1"};
    QString m_driver{"e1000e"};
    QString m_interface{"eno1"};
    QString m_udi{"/sys/devices/pci0000:00/0000:00:1f.6/net/eno1"};
    quint32 m_deviceType{1};
    quint32 m_interfaceFlags{3};
    quint32 m_state{100};

    QList<QDBusObjectPath> availableConnections() { return m_availableConnections; }
    bool autoconnect() { return m_autoconnect; }
    void setAutoconnect(const bool autoconnect) { m_autoconnect = autoconnect; }
    bool managed() { return m_managed; }
    void setManaged(const bool managed) { m_managed = managed; }
    QDBusObjectPath activeConnection() { return m_activeConnection; }
    QDBusObjectPath DHCP4Config() { return m_DHCP4Config; }
    QDBusObjectPath DHCP6Config() { return m_DHCP6Config; }
    QDBusObjectPath IPv4Config() { return m_IPv4Config; }
    QDBusObjectPath IPv6Config() { return m_IPv6Config; }
    QString driver() { return m_driver; }
    QString interface() { return m_interface; }
    QString udi() { return m_udi; }
    quint32 deviceType() { return m_deviceType; }
    quint32 interfaceFlags() { return m_interfaceFlags; }
    quint32 state() { return m_state; }

    bool m_disconnectTrigger{false};

public:
    Q_SCRIPTABLE void Disconnect() { m_disconnectTrigger = true; }

private:
    void registerService();
    void unregisterService();
    const QString Service{"com.deepin.FakeNetworkManager"};
    const QString Path{"/com/deepin/FakeNetworkManager/Devices/2"};
};

#endif
