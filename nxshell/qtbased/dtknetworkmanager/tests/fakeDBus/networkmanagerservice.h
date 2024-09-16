// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef NETWORKMANAGERSERVICE_H
#define NETWORKMANAGERSERVICE_H

#include "dnetworkmanagertypes.h"
#include <QDBusObjectPath>
#include <QDBusMetaType>
#include <QString>
#include <QList>

DNETWORKMANAGER_USE_NAMESPACE

class FakeNetworkManagerService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.FakeNetworkManager")
public:
    explicit FakeNetworkManagerService(QObject *parent = nullptr);
    ~FakeNetworkManagerService() override;

    Q_PROPERTY(bool NetworkingEnabled READ networkingEnabled)
    Q_PROPERTY(bool WirelessEnabled READ wirelessEnabled WRITE setWirelessEnabled)
    Q_PROPERTY(bool WirelessHardwareEnabled READ wirelessHardwareEnabled)
    Q_PROPERTY(QList<QDBusObjectPath> ActiveConnections READ activeConnections)
    Q_PROPERTY(QDBusObjectPath PrimaryConnection READ primaryConnection)
    Q_PROPERTY(QString PrimaryConnectionType READ primaryConnectionType)
    Q_PROPERTY(quint32 State READ state)
    Q_PROPERTY(quint32 Connectivity READ connectivity)

    bool networkingEnabled() const { return m_networkingEnabled; };
    bool wirelessEnabled() const { return m_wirelessEnabled; };
    void setWirelessEnabled(const bool enable) { m_wirelessEnabled = enable; };
    bool wirelessHardwareEnabled() const { return m_wirelessHardwareEnabled; };
    QList<QDBusObjectPath> activeConnections() const { return m_activeConnections; };
    QDBusObjectPath primaryConnection() const { return m_primaryConnection; };
    QString primaryConnectionType() const { return m_primaryConnectionType; };
    quint32 state() const { return m_state; };
    quint32 connectivity() const { return m_connectivity; };

    bool m_networkingEnabled{true};
    bool m_wirelessEnabled{true};
    bool m_wirelessHardwareEnabled{true};
    QList<QDBusObjectPath> m_activeConnections{QDBusObjectPath{"/com/deepin/FakeNetworkManager/ActiveConnection/1"}};
    QDBusObjectPath m_primaryConnection{"/com/deepin/FakeNetworkManager/ActiveConnection/1"};
    QString m_primaryConnectionType{"802-3-ethernet"};
    quint32 m_state{70};
    quint32 m_connectivity{4};

    bool m_getDevicesTrigger{false};
    bool m_activateConnectionTrigger{false};
    bool m_addAndActivateConnectionTrigger{false};
    bool m_deactivateConnectionTrigger{false};
    bool m_enableTrigger{false};
    bool m_getPermissionsTrigger{false};
    bool m_checkConnectivityTrigger{false};

public Q_SLOTS:
    Q_SCRIPTABLE QList<QDBusObjectPath> GetDevices()
    {
        m_getDevicesTrigger = true;
        return {};
    }

    Q_SCRIPTABLE QDBusObjectPath ActivateConnection(QDBusObjectPath, QDBusObjectPath, QDBusObjectPath)
    {
        m_activateConnectionTrigger = true;
        return {};
    }

    Q_SCRIPTABLE QDBusObjectPath AddAndActivateConnection(SettingDesc, QDBusObjectPath, QDBusObjectPath, QDBusObjectPath &path)
    {
        m_addAndActivateConnectionTrigger = true;
        path = {};
        return {};
    }

    Q_SCRIPTABLE void DeactivateConnection(QDBusObjectPath) { m_deactivateConnectionTrigger = true; }

    Q_SCRIPTABLE void Enable(const bool) { m_enableTrigger = true; }

    Q_SCRIPTABLE QMap<QString, QString> GetPermissions()
    {
        m_getPermissionsTrigger = true;
        return {};
    }

    Q_SCRIPTABLE quint32 CheckConnectivity()
    {
        m_checkConnectivityTrigger = true;
        return 0;
    }

private:
    void registerService();
    void unregisterService();
    const QString Service{"com.deepin.FakeNetworkManager"};
    const QString Path{"/com/deepin/FakeNetworkManager"};
};

#endif
