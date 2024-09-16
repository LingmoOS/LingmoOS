// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DNETWORKMANAGER_H
#define DNETWORKMANAGER_H

#include "dnetworkmanagertypes.h"
#include "dnmsetting.h"
#include <DExpected>
#include <QSharedPointer>

DNETWORKMANAGER_BEGIN_NAMESPACE

class DNetworkManagerPrivate;
class DDevice;
class DActiveConnection;

using DCORE_NAMESPACE::DExpected;

class DNetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit DNetworkManager(QObject *parent = nullptr);
    ~DNetworkManager() override;

    Q_PROPERTY(bool networkingEnabled READ networkingEnabled NOTIFY networkingEnabledChanged)
    Q_PROPERTY(bool wirelessEnabled READ wirelessEnabled WRITE setWirelessEnabled NOTIFY wirelessEnabledChanged)
    Q_PROPERTY(bool wirelessHardwareEnabled READ wirelessHardwareEnabled NOTIFY wirelessHardwareEnabledChanged)
    Q_PROPERTY(QList<quint64> activeConnections READ getActiveConnectionsIdList NOTIFY activeConnectionsChanged)
    Q_PROPERTY(quint64 primaryConnection READ primaryConnection NOTIFY primaryConnectionChanged)
    Q_PROPERTY(DNMSetting::SettingType primaryConnectionType READ primaryConnectionType NOTIFY primaryConnectionTypeChanged)
    Q_PROPERTY(NMState state READ state NOTIFY StateChanged)
    Q_PROPERTY(NMConnectivityState connectivity READ connectivity NOTIFY connectivityChanged)

    bool networkingEnabled() const;
    bool wirelessEnabled() const;
    void setWirelessEnabled(const bool enable) const;
    bool wirelessHardwareEnabled() const;
    QList<quint64> getActiveConnectionsIdList() const;
    quint64 primaryConnection() const;
    DNMSetting::SettingType primaryConnectionType() const;
    NMState state() const;
    NMConnectivityState connectivity() const;

public Q_SLOTS:

    DExpected<QList<quint64>> getDeviceIdList() const;
    DExpected<quint64> activateConnection(const quint64 connId, const quint64 deviceId, const qint64 objId = 0) const;
    DExpected<NewConn> addAndActivateConnection(const SettingDesc &conn, const quint64 deviceId, const qint64 objId = 0) const;
    DExpected<void> deactivateConnection(const quint64 activeConnId) const;
    DExpected<void> enable(const bool enabled) const;
    DExpected<QMap<QString, QString>> permissions() const;
    DExpected<NMConnectivityState> checkConnectivity() const;
    DExpected<QSharedPointer<DDevice>> getDeviceObject(const quint64 id) const;
    DExpected<QSharedPointer<DActiveConnection>> getActiveConnectionObject(const quint64 id) const;

Q_SIGNALS:

    void networkingEnabledChanged(const bool enable);
    void wirelessEnabledChanged(const bool enable);
    void wirelessHardwareEnabledChanged(const bool enable);
    void activeConnectionsChanged(const QList<quint64> &connIds);
    void primaryConnectionChanged(const quint64 connId);
    void primaryConnectionTypeChanged(const DNMSetting::SettingType &type);
    void connectivityChanged(const NMConnectivityState connState);
    void DeviceAdded(const quint64 deviceId);
    void DeviceRemoved(const quint64 deviceId);
    void CheckPermissions();
    void StateChanged(const NMState state);

private:
    QScopedPointer<DNetworkManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DNetworkManager)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
