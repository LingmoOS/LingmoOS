// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBLUETOOTHOBEXAGENT_H
#define DBLUETOOTHOBEXAGENT_H

#include <QDBusObjectPath>
#include "dtkbluetooth_global.h"
#include "dbluetoothrequest.h"

DBLUETOOTH_BEGIN_NAMESPACE

class DObexTransfer;
class DObexSession;

class DObexAgent : public QObject
{
    Q_OBJECT
public:
    explicit DObexAgent(QObject *parent = nullptr);

    virtual QDBusObjectPath agentPath() const = 0;
    ~DObexAgent() override = default;

    virtual void
    authorizePush(QSharedPointer<DObexTransfer> transfer, QSharedPointer<DObexSession> session, const DRequest<QString> &request);
    virtual void cancel();
    virtual void release();
};

DBLUETOOTH_END_NAMESPACE

#endif
