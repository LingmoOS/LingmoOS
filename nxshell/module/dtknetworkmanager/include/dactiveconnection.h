// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DACTIVECONNECTION_H
#define DACTIVECONNECTION_H

#include "dnetworkmanagertypes.h"
#include "settings/dnmsetting.h"
#include <QUuid>

DNETWORKMANAGER_BEGIN_NAMESPACE

class DActiveConnectionPrivate;

class DActiveConnection : public QObject
{
    Q_OBJECT
public:
    explicit DActiveConnection(const quint64 activeConnId, QObject *parent = nullptr);
    explicit DActiveConnection(DActiveConnectionPrivate &other, QObject *parent = nullptr);
    ~DActiveConnection() override;

    Q_PROPERTY(QList<quint64> devices READ devices NOTIFY devicesChanged)
    Q_PROPERTY(bool vpn READ vpn NOTIFY vpnChanged)
    Q_PROPERTY(quint64 connection READ connection NOTIFY connectionChanged)
    Q_PROPERTY(quint64 DHCP4Config READ DHCP4Config NOTIFY DHCP4ConfigChanged)
    Q_PROPERTY(quint64 DHCP6Config READ DHCP6Config NOTIFY DHCP6ConfigChanged)
    Q_PROPERTY(quint64 IP4Config READ IP4Config NOTIFY IP4ConfigChanged)
    Q_PROPERTY(quint64 IP6Config READ IP6Config NOTIFY IP6ConfigChanged)
    Q_PROPERTY(quint64 specificObject READ specificObject NOTIFY specificObjectChanged)
    Q_PROPERTY(QString connectionId READ connectionId NOTIFY connectionIdChanged)
    Q_PROPERTY(DNMSetting::SettingType connectionType READ connectionType NOTIFY connectionTypeChanged)
    Q_PROPERTY(QUuid UUID READ UUID NOTIFY UUIDChanged)
    Q_PROPERTY(NMActiveConnectionState connectionState READ connectionState NOTIFY connectionStateChanged)

    QList<quint64> devices() const;
    bool vpn() const;
    quint64 connection() const;
    quint64 DHCP4Config() const;
    quint64 DHCP6Config() const;
    quint64 IP4Config() const;
    quint64 IP6Config() const;
    quint64 specificObject() const;
    QString connectionId() const;
    DNMSetting::SettingType connectionType() const;
    QUuid UUID() const;
    NMActiveConnectionState connectionState() const;

Q_SIGNALS:
    void devicesChanged(const QList<quint64> &devices);
    void vpnChanged(const bool vpn);
    void connectionChanged(const quint64 &connection);
    void DHCP4ConfigChanged(const quint64 &config);
    void DHCP6ConfigChanged(const quint64 &config);
    void IP4ConfigChanged(const quint64 &config);
    void IP6ConfigChanged(const quint64 &config);
    void specificObjectChanged(const quint64 &specObj);
    void connectionIdChanged(const QString &id);
    void connectionTypeChanged(const DNMSetting::SettingType type);
    void UUIDChanged(const QUuid &UUID);
    void connectionStateChanged(const NMActiveConnectionState state, const NMActiveConnectionStateReason reason);

protected:
    QScopedPointer<DActiveConnectionPrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(DActiveConnection)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
