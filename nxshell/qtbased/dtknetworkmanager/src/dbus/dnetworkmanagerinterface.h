// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DNETWORKMANAGERINTERFACE_H
#define DNETWORKMANAGERINTERFACE_H

#include "dnetworkmanager_global.h"
#include "dnetworkmanagertypes.h"
#include <DDBusInterface>
#include <QSharedPointer>
#include <QDBusPendingReply>
#include <QString>
#include <QMap>
#include <QVariant>
#include <QList>

DNETWORKMANAGER_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;

class DNetworkManagerInterface : public QObject
{
    Q_OBJECT
public:
    explicit DNetworkManagerInterface(QObject *parent = nullptr);
    ~DNetworkManagerInterface() override = default;

    Q_PROPERTY(bool networkingEnabled READ networkingEnabled NOTIFY NetworkingEnabledChanged)
    Q_PROPERTY(bool wirelessEnabled READ wirelessEnabled WRITE setWirelessEnabled NOTIFY WirelessEnabledChanged)
    Q_PROPERTY(bool wirelessHardwareEnabled READ wirelessHardwareEnabled NOTIFY WirelessHardwareEnabledChanged)
    Q_PROPERTY(QList<QDBusObjectPath> ActiveConnections READ activeConnections NOTIFY ActiveConnectionsChanged)
    Q_PROPERTY(QDBusObjectPath primaryConnection READ primaryConnection NOTIFY PrimaryConnectionChanged)
    Q_PROPERTY(QString primaryConnectionType READ primaryConnectionType NOTIFY PrimaryConnectionTypeChanged)
    Q_PROPERTY(quint32 state READ state NOTIFY StateChanged)
    Q_PROPERTY(quint32 Connectivity READ connectivity NOTIFY ConnectivityChanged)

    bool networkingEnabled() const;
    bool wirelessEnabled() const;
    void setWirelessEnabled(const bool enable) const;
    bool wirelessHardwareEnabled() const;
    QList<QDBusObjectPath> activeConnections() const;
    QDBusObjectPath primaryConnection() const;
    QString primaryConnectionType() const;
    quint32 state() const;
    quint32 connectivity() const;

public Q_SLOTS:
    QDBusPendingReply<QList<QDBusObjectPath>> getDevices();
    QDBusPendingReply<QDBusObjectPath>
    activateConnection(const QByteArray &connection, const QByteArray &device, const QByteArray &specObj);
    QDBusPendingReply<QDBusObjectPath, QDBusObjectPath>
    addAndActivateConnection(const SettingDesc &connection, const QByteArray &device, const QByteArray &specObj);
    QDBusPendingReply<void> deactivateConnection(const QByteArray &activateConnection);
    QDBusPendingReply<void> enable(const bool enable);
    QDBusPendingReply<QMap<QString, QString>> getPermissions();
    QDBusPendingReply<quint32> checkConnectivity();

Q_SIGNALS:
    void NetworkingEnabledChanged(const bool enable);
    void WirelessEnabledChanged(const bool enable);
    void WirelessHardwareEnabledChanged(const bool enable);
    void ActiveConnectionsChanged(const QList<QDBusObjectPath> &conn);
    void PrimaryConnectionChanged(const QDBusObjectPath &path);
    void PrimaryConnectionTypeChanged(const QString &type);
    void ConnectivityChanged(const quint32 con);

    void DeviceAdded(const QDBusObjectPath &device);
    void DeviceRemoved(const QDBusObjectPath &device);
    void CheckPermissions();
    void StateChanged(quint32 state);

private:
    DDBusInterface *m_inter{nullptr};
};

DNETWORKMANAGER_END_NAMESPACE

#endif
