// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DDEVICEINTERFACE_H
#define DDEVICEINTERFACE_H

#include "dtkbluetooth_global.h"
#include "dbluetoothdbustypes.h"
#include <QObject>
#include <QDBusObjectPath>
#include <DDBusInterface>
#include <QDBusPendingReply>
#include <QBluetoothUuid>

DBLUETOOTH_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;

class DDeviceInterface : public QObject
{
    Q_OBJECT
public:
    explicit DDeviceInterface(const QString &path, QObject *parent = nullptr);
    ~DDeviceInterface() override = default;

    Q_PROPERTY(bool blocked READ blocked WRITE setBlocked NOTIFY blockedChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(bool legacyPairing READ legacyPairing NOTIFY legacyPairingChanged)
    Q_PROPERTY(bool paired READ paired NOTIFY pairedChanged)
    Q_PROPERTY(bool servicesResolved READ servicesResolved NOTIFY servicesResolvedChanged)
    Q_PROPERTY(bool trusted READ trusted WRITE setTrusted NOTIFY trustedChanged)
    Q_PROPERTY(QString adapter READ adapter CONSTANT)
    Q_PROPERTY(QString address READ address CONSTANT)
    Q_PROPERTY(QString addressType READ addressType NOTIFY addressTypeChanged)
    Q_PROPERTY(QString alias READ alias NOTIFY aliasChanged)
    Q_PROPERTY(quint32 Class READ Class CONSTANT)
    Q_PROPERTY(quint16 appearance READ appearance CONSTANT)
    Q_PROPERTY(QStringList UUIDs READ UUIDs CONSTANT)
    Q_PROPERTY(QString icon READ icon CONSTANT)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(qint16 RSSI READ RSSI NOTIFY RSSIChanged)

    bool blocked() const;
    void setBlocked(bool blocked);
    bool connected() const;
    bool legacyPairing() const;
    bool paired() const;
    bool servicesResolved() const;
    bool trusted() const;
    void setTrusted(bool trusted);
    quint32 Class() const;
    quint16 appearance() const;
    QString adapter() const;
    QString address() const;
    QString addressType() const;
    QString alias() const;
    QStringList UUIDs() const;
    QString icon() const;
    QString name() const;
    qint16 RSSI() const;
    bool isValid() const;

public Q_SLOTS:
    QDBusPendingReply<void> disconnect();
    QDBusPendingReply<void> cancelPairing();
    QDBusPendingReply<void> connect();
    QDBusPendingReply<void> pair();

Q_SIGNALS:
    void blockedChanged(bool blocked);
    void connectedChanged(bool connected);
    void legacyPairingChanged(bool paired);
    void pairedChanged(bool servicesResolved);
    void servicesResolvedChanged(bool servicesResolved);
    void trustedChanged(bool trusted);
    void addressTypeChanged(const QString &addressType);
    void aliasChanged(const QString &alias);
    void nameChanged(const QString &name);
    void RSSIChanged(qint16 rssi);

    void removed();

private:
    DDBusInterface *m_inter{nullptr};
};

DBLUETOOTH_END_NAMESPACE

#endif
