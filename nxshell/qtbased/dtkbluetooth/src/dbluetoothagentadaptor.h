// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBLUETOOTHAGENTADAPTOR_H
#define DBLUETOOTHAGENTADAPTOR_H

#include "dbluetoothdevice.h"
#include "dtkbluetooth_global.h"
#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>
#include <QDBusMessage>

DBLUETOOTH_BEGIN_NAMESPACE

class DAgent;

class DAgentAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Agent1")

public:
    explicit DAgentAdaptor(DAgent *parent);

public Q_SLOTS:
    QString RequestPinCode(const QDBusObjectPath &device, const QDBusMessage &msg);
    Q_NOREPLY void DisplayPinCode(const QDBusObjectPath &device, const QString &pinCode);
    quint32 RequestPasskey(const QDBusObjectPath &device, const QDBusMessage &msg);
    Q_NOREPLY void DisplayPasskey(const QDBusObjectPath &device, quint32 passkey, quint16 entered);
    void RequestConfirmation(const QDBusObjectPath &device, quint32 passkey, const QDBusMessage &msg);
    void RequestAuthorization(const QDBusObjectPath &device, const QDBusMessage &msg);
    void AuthorizeService(const QDBusObjectPath &device, const QString &uuid, const QDBusMessage &msg);

    Q_NOREPLY void Cancel();
    Q_NOREPLY void Release();

private:
    QString passkeyToString(quint32 passkey) const;
    QSharedPointer<DDevice> deviceFromUbi(const QDBusObjectPath &device) const;
    DAgent *m_agent{nullptr};
};

DBLUETOOTH_END_NAMESPACE

#endif
