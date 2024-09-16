// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DDEVICEINTERFACE_H
#define DDEVICEINTERFACE_H

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

class DDeviceInterface : public QObject
{
    Q_OBJECT
public:
    explicit DDeviceInterface(const QByteArray &path, QObject *parent = nullptr);
    ~DDeviceInterface() override = default;

    Q_PROPERTY(QList<QDBusObjectPath> availableConnections READ availableConnections NOTIFY AvailableConnectionsChanged)
    Q_PROPERTY(bool autoconnect READ autoconnect WRITE setAutoconnect NOTIFY AutoconnectChanged)
    Q_PROPERTY(bool managed READ managed WRITE setManaged NOTIFY ManagedChanged)
    Q_PROPERTY(QDBusObjectPath activeConnection READ activeConnection NOTIFY ActiveConnectionChanged)
    Q_PROPERTY(QDBusObjectPath DHCP4Config READ DHCP4Config NOTIFY Dhcp4ConfigChanged)
    Q_PROPERTY(QDBusObjectPath DHCP6Config READ DHCP6Config NOTIFY Dhcp4ConfigChanged)
    Q_PROPERTY(QDBusObjectPath IPv4Config READ IPv4Config NOTIFY IP4ConfigChanged)
    Q_PROPERTY(QDBusObjectPath IPv6Config READ IPv6Config NOTIFY IP6ConfigChanged)
    Q_PROPERTY(QString driver READ driver NOTIFY DriverChanged)
    Q_PROPERTY(QString interface READ interface NOTIFY InterfaceChanged)
    Q_PROPERTY(QString udi READ udi NOTIFY UdiChanged)
    Q_PROPERTY(quint32 deviceType READ deviceType NOTIFY DeviceTypeChanged)
    Q_PROPERTY(quint32 interfaceFlags READ interfaceFlags NOTIFY InterfaceFlagsChanged)
    Q_PROPERTY(quint32 state READ state NOTIFY StateChanged)
    Q_PROPERTY(quint32 refreshRateMs READ refreshRateMs WRITE setRefreshRateMs NOTIFY RefreshRateMsChanged)
    Q_PROPERTY(quint32 rxBytes READ rxBytes NOTIFY RxBytesChanged)
    Q_PROPERTY(quint32 txBytes READ txBytes NOTIFY TxBytesChanged)

    QList<QDBusObjectPath> availableConnections() const;
    bool autoconnect() const;
    void setAutoconnect(const bool autoconnect) const;
    bool managed() const;
    void setManaged(const bool managed) const;
    QDBusObjectPath activeConnection() const;
    QDBusObjectPath DHCP4Config() const;
    QDBusObjectPath DHCP6Config() const;
    QDBusObjectPath IPv4Config() const;
    QDBusObjectPath IPv6Config() const;
    QString driver() const;
    QString interface() const;
    QString udi() const;
    quint32 deviceType() const;
    quint32 interfaceFlags() const;
    quint32 state() const;
    quint32 refreshRateMs() const;
    void setRefreshRateMs(const quint32 newRate) const;
    quint64 rxBytes() const;
    quint64 txBytes() const;

public Q_SLOTS:

    QDBusPendingReply<void> disconnect() const;

Q_SIGNALS:
    void StateChanged(const quint32 newSatate, const quint32 oldState, const quint32 reason);
    void AvailableConnectionsChanged(const QList<QDBusObjectPath> &conns);
    void AutoconnectChanged(const bool enabled);
    void ManagedChanged(const bool enabled);
    void ActiveConnectionChanged(const QDBusObjectPath &conn);
    void Dhcp4ConfigChanged(const QDBusObjectPath &config);
    void Dhcp6ConfigChanged(const QDBusObjectPath &config);
    void IP4ConfigChanged(const QDBusObjectPath &config);
    void IP6ConfigChanged(const QDBusObjectPath &config);
    void DriverChanged(const QString &driver);
    void InterfaceChanged(const QString &ifc);
    void UdiChanged(const QString &udi);
    void DeviceTypeChanged(const quint32 type);
    void InterfaceFlagsChanged(const quint32 flags);
    void RefreshRateMsChanged(const quint32 rate);
    void RxBytesChanged(const quint64 bytes);
    void TxBytesChanged(const quint64 bytes);

private:
    DDBusInterface *m_deviceInter{nullptr};
    DDBusInterface *m_statisticsInter{nullptr};
};

DNETWORKMANAGER_END_NAMESPACE

#endif
