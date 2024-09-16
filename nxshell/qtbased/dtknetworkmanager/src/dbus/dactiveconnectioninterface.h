// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DACTIVECONNECTIONINTERFACE_H
#define DACTIVECONNECTIONINTERFACE_H

#include "dnetworkmanager_global.h"
#include "dnetworkmanagertypes.h"
#include <DDBusInterface>
#include <QList>
#include <QDBusObjectPath>

DNETWORKMANAGER_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;

class DActiveConnectionInterface : public QObject
{
    Q_OBJECT
public:
    explicit DActiveConnectionInterface(const QByteArray &Path, QObject *parent = nullptr);
    ~DActiveConnectionInterface() override = default;

    Q_PROPERTY(QList<QDBusObjectPath> devices READ devices NOTIFY DevicesChanged)
    Q_PROPERTY(bool vpn READ vpn NOTIFY VpnChanged)
    Q_PROPERTY(QDBusObjectPath connection READ connection NOTIFY ConnectionChanged)
    Q_PROPERTY(QDBusObjectPath DHCP4Config READ DHCP4Config NOTIFY Dhcp4ConfigChanged)
    Q_PROPERTY(QDBusObjectPath DHCP6Config READ DHCP6Config NOTIFY Dhcp6ConfigChanged)
    Q_PROPERTY(QDBusObjectPath IP4Config READ IP4Config NOTIFY IP4ConfigChanged)
    Q_PROPERTY(QDBusObjectPath IP6Config READ IP6Config NOTIFY IP6ConfigChanged)
    Q_PROPERTY(QDBusObjectPath specificObject READ specificObject NOTIFY SpecificObjectChanged)
    Q_PROPERTY(QString id READ id NOTIFY IdChanged)
    Q_PROPERTY(QString uuid READ uuid NOTIFY UuidChanged)
    Q_PROPERTY(QString type READ type NOTIFY TypeChanged)
    Q_PROPERTY(quint32 state READ state NOTIFY StateChanged)

    QList<QDBusObjectPath> devices() const;
    bool vpn() const;
    QDBusObjectPath connection() const;
    QDBusObjectPath DHCP4Config() const;
    QDBusObjectPath DHCP6Config() const;
    QDBusObjectPath IP4Config() const;
    QDBusObjectPath IP6Config() const;
    QDBusObjectPath specificObject() const;
    QString id() const;
    QString type() const;
    QString uuid() const;
    quint32 state() const;

Q_SIGNALS:
    void DevicesChanged(const QList<QDBusObjectPath> &devices);
    void VpnChanged(const bool vpn);
    void ConnectionChanged(const QDBusObjectPath &connection);
    void Dhcp4ConfigChanged(const QDBusObjectPath &config);
    void Dhcp6ConfigChanged(const QDBusObjectPath &config);
    void IP4ConfigChanged(const QDBusObjectPath &config);
    void IP6ConfigChanged(const QDBusObjectPath &config);
    void SpecificObjectChanged(const QDBusObjectPath &specObj);
    void IdChanged(const QString &id);
    void UuidChanged(const QString &uuid);
    void TypeChanged(const QString &type);

    void StateChanged(const quint32 state, const quint32 reason);

private:
    DDBusInterface *m_inter{nullptr};
};

DNETWORKMANAGER_END_NAMESPACE

#endif
