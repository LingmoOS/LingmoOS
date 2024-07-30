/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_FAKE_NETWORK_H
#define NETWORKMANAGERQT_FAKE_NETWORK_H

#include <QObject>

#include <QDBusObjectPath>

#include "../device.h"
#include "../generictypes.h"

#include "activeconnection.h"
#include "device.h"
#include "settings.h"

class FakeNetwork : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.fakenetwork")
public:
    explicit FakeNetwork(QObject *parent = nullptr);
    ~FakeNetwork() override;

    Q_PROPERTY(QDBusObjectPath ActivatingConnection READ activatingConnection)
    Q_PROPERTY(QList<QDBusObjectPath> ActiveConnections READ activeConnections)
    Q_PROPERTY(uint Connectivity READ connectivity)
    Q_PROPERTY(QList<QDBusObjectPath> Devices READ devices)
    Q_PROPERTY(bool NetworkingEnabled READ networkingEnabled)
    Q_PROPERTY(QDBusObjectPath PrimaryConnection READ primaryConnection)
    Q_PROPERTY(uint State READ state)
    Q_PROPERTY(QString Version READ version)
    Q_PROPERTY(bool WimaxEnabled READ wimaxEnabled WRITE setWimaxEnabled)
    Q_PROPERTY(bool WimaxHardwareEnabled READ wimaxHardwareEnabled)
    Q_PROPERTY(bool WirelessEnabled READ wirelessEnabled WRITE setWirelessEnabled)
    Q_PROPERTY(bool WirelessHardwareEnabled READ wirelessHardwareEnabled)
    Q_PROPERTY(bool WwanEnabled READ wwanEnabled WRITE setWwanEnabled)
    Q_PROPERTY(bool WwanHardwareEnabled READ wwanHardwareEnabled)

    QDBusObjectPath activatingConnection() const;
    QList<QDBusObjectPath> activeConnections() const;
    uint connectivity() const;
    QList<QDBusObjectPath> devices() const;
    bool networkingEnabled() const;
    QDBusObjectPath primaryConnection() const;
    uint state() const;
    QString version() const;
    bool wimaxEnabled() const;
    void setWimaxEnabled(bool enabled);
    bool wimaxHardwareEnabled() const;
    void setWimaxHardwareEnabled(bool enabled);
    bool wirelessEnabled() const;
    void setWirelessEnabled(bool enabled);
    bool wirelessHardwareEnabled() const;
    void setWirelessHardwareEnabled(bool enabled);
    bool wwanEnabled() const;
    void setWwanEnabled(bool enabled);
    bool wwanHardwareEnabled() const;

    /* Not part of DBus interface */
    void addDevice(Device *device);
    void removeDevice(Device *device);
    void registerService();
    void unregisterService();

private Q_SLOTS:
    void onConnectionAdded(const QDBusObjectPath &connection);
    void onConnectionRemoved(const QDBusObjectPath &connection);
    void removeActiveConnection(const QDBusObjectPath &activeConnection);
    void updateConnectingState();
    void updateDeactivatingState();

public Q_SLOTS:
    Q_SCRIPTABLE QDBusObjectPath ActivateConnection(const QDBusObjectPath &connection, const QDBusObjectPath &device, const QDBusObjectPath &specific_object);
    Q_SCRIPTABLE uint CheckConnectivity() const;
    Q_SCRIPTABLE void DeactivateConnection(const QDBusObjectPath &active_connection);
    Q_SCRIPTABLE QDBusObjectPath GetDeviceByIpIface(const QString &iface);
    Q_SCRIPTABLE QList<QDBusObjectPath> GetDevices() const;

Q_SIGNALS:
    Q_SCRIPTABLE void DeviceAdded(const QDBusObjectPath &device_path);
    Q_SCRIPTABLE void DeviceRemoved(const QDBusObjectPath &device_path);
    Q_SCRIPTABLE void PropertiesChanged(const QVariantMap &properties);
    Q_SCRIPTABLE void StateChanged(uint state);

private:
    QDBusObjectPath m_activatingConnection;
    QMap<QDBusObjectPath, ActiveConnection *> m_activeConnections;
    uint m_connectivity;
    QMap<QDBusObjectPath, Device *> m_devices;
    bool m_networkingEnabled;
    QDBusObjectPath m_primaryConnection;
    uint m_state;
    QString m_version;
    bool m_wimaxEnabled;
    bool m_wimaxHardwareEnabled;
    bool m_wirelessEnabled;
    bool m_wirelessHardwareEnabled;
    bool m_wwanEnabled;
    bool m_wwanHardwareEnabled;

    /* Not part of DBus interface */
    QString m_activatedDevice;
    QString m_deactivatedDevice;
    int m_activeConnectionsCounter;
    int m_deviceCounter;
    Settings *m_settings;
};

#endif
