// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBLUETOOTHAGENT_H
#define DBLUETOOTHAGENT_H

#include <DExpected>
#include <QDBusObjectPath>
#include "dtkbluetooth_global.h"
#include "dbluetoothrequest.h"

DBLUETOOTH_BEGIN_NAMESPACE

class DDevice;

class DAgent : public QObject
{
    Q_OBJECT
public:
    enum Capability { DisplayOnly, DisplayYesNo, KeyboardOnly, NoInputNoOutput };

    Q_ENUM(Capability)

    explicit DAgent(QObject *parent = nullptr);
    ~DAgent() override = default;
    virtual QDBusObjectPath agentPath() const = 0;
    virtual Capability capability() const;

public Q_SLOTS:
    virtual void requestPinCode(QSharedPointer<DDevice> device, const DRequest<QString> &request);
    virtual void displayPinCode(QSharedPointer<DDevice> device, const QString &pinCode);
    virtual void requestPasskey(QSharedPointer<DDevice> device, const DRequest<quint32> &request);
    virtual void displayPasskey(QSharedPointer<DDevice> device, const QString &passkey, const QString &entered);
    virtual void requestConfirmation(QSharedPointer<DDevice> device, const QString &passkey, const DRequest<> &request);
    virtual void requestAuthorization(QSharedPointer<DDevice> device, const DRequest<> &request);
    virtual void authorizeService(QSharedPointer<DDevice> device, const QString &uuid, const DRequest<> &request);
    virtual void cancel();
    virtual void release();
};

DBLUETOOTH_END_NAMESPACE

#endif
