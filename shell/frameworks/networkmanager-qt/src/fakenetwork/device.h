/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_FAKE_NETWORK_DEVICE_H
#define NETWORKMANAGERQT_FAKE_NETWORK_DEVICE_H

#include <QObject>

#include <QDBusObjectPath>

#include "../device.h"
#include "../generictypes.h"

class Device : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.fakenetwork.Device")
public:
    explicit Device(QObject *parent = nullptr);
    ~Device() override;

    Q_PROPERTY(QDBusObjectPath ActiveConnection READ activeConnection)
    Q_PROPERTY(bool Autoconnect READ autoconnect WRITE setAutoconnect)
    Q_PROPERTY(QList<QDBusObjectPath> AvailableConnections READ availableConnections)
    Q_PROPERTY(uint Capabilities READ capabilities)
    Q_PROPERTY(uint DeviceType READ deviceType)
    Q_PROPERTY(QDBusObjectPath Dhcp4Config READ dhcp4Config)
    Q_PROPERTY(QDBusObjectPath Dhcp6Config READ dhcp6Config)
    Q_PROPERTY(QString Driver READ driver)
    Q_PROPERTY(QString DriverVersion READ driverVersion)
    Q_PROPERTY(bool FirmwareMissing READ firmwareMissing)
    Q_PROPERTY(QString FirmwareVersion READ firmwareVersion)
    Q_PROPERTY(QString Interface READ interface)
    Q_PROPERTY(int Ip4Address READ ip4Address)
    Q_PROPERTY(QDBusObjectPath Ip4Config READ ip4Config)
    Q_PROPERTY(QDBusObjectPath Ip6Config READ ip6Config)
    Q_PROPERTY(QString IpInterface READ ipInterface)
    Q_PROPERTY(uint InterfaceFlags READ interfaceFlags)
    Q_PROPERTY(bool Managed READ managed)
    Q_PROPERTY(uint Mtu READ mtu)
    Q_PROPERTY(uint State READ state)
    Q_PROPERTY(DeviceDBusStateReason StateReason READ stateReason)
    Q_PROPERTY(QString Udi READ udi)

    QDBusObjectPath activeConnection() const;
    bool autoconnect() const;
    void setAutoconnect(bool autoconnect);
    QList<QDBusObjectPath> availableConnections() const;
    uint capabilities() const;
    uint deviceType() const;
    QDBusObjectPath dhcp4Config() const;
    QDBusObjectPath dhcp6Config() const;
    QString driver() const;
    QString driverVersion() const;
    bool firmwareMissing() const;
    QString firmwareVersion() const;
    QString interface() const;
    int ip4Address() const;
    QDBusObjectPath ip4Config() const;
    QDBusObjectPath ip6Config() const;
    QString ipInterface() const;
    uint interfaceFlags() const;
    bool managed() const;
    uint mtu() const;
    uint state() const;
    DeviceDBusStateReason stateReason() const;
    QString udi() const;

    /* Not part of DBus interface */
    void addAvailableConnection(const QDBusObjectPath &availableConnection);
    void removeAvailableConnection(const QDBusObjectPath &availableConnection);
    void setActiveConnection(const QString &activeConnection);
    void setCapabilities(uint capabilities);
    QString deviceInterface() const;
    QString devicePath() const;
    void setDevicePath(const QString &devicePath);
    void setDeviceType(uint deviceType);
    void setDhcp4Config(const QString &config);
    void setDhcp6Config(const QString &config);
    void setDriver(const QString &driver);
    void setDriverVersion(const QString &driverVersion);
    void setFirmwareMissing(bool firmwareMissing);
    void setFirmwareVersion(const QString &firmwareVersion);
    void setInterface(const QString &interface);
    void setInterfaceFlags(uint interfaceFlags);
    void setIpv4Address(int address);
    void setIp4Config(const QString &config);
    void setIp6Config(const QString &config);
    void setIpInterface(const QString &interface);
    void setManaged(bool managed);
    void setMtu(uint mtu);
    virtual void setState(uint state);
    void setStateReason(const DeviceDBusStateReason &reason);
    void setUdi(const QString &udi);

public Q_SLOTS:
    Q_SCRIPTABLE void Disconnect();

Q_SIGNALS:
    void activeConnectionRemoved(const QDBusObjectPath &activeConnection);
    Q_SCRIPTABLE void StateChanged(uint new_state, uint old_state, uint reason);

private:
    QDBusObjectPath m_activeConnection;
    bool m_autoconnect;
    QList<QDBusObjectPath> m_availableConnections;
    uint m_capabilities;
    QString m_devicePath;
    uint m_deviceType;
    QDBusObjectPath m_dhcp4Config;
    QDBusObjectPath m_dhcp6Config;
    QString m_driver;
    QString m_driverVersion;
    bool m_firmwareMissing;
    QString m_firmwareVersion;
    QString m_interface;
    uint m_interfaceFlags;
    int m_ip4Address;
    QDBusObjectPath m_ip4Config;
    QDBusObjectPath m_ip6Config;
    QString m_ipInterface;
    bool m_managed;
    uint m_mtu;
    QString m_physicalPortId;
    uint m_state;
    DeviceDBusStateReason m_stateReason;
    QString m_udi;
};

#endif
