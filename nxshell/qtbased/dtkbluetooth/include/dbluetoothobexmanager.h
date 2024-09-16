// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBLUETOOTHOBEXMANAGER_H
#define DBLUETOOTHOBEXMANAGER_H

#include "dbluetoothtypes.h"
#include <DExpected>
#include <DObject>

DBLUETOOTH_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DExpected;
using DTK_CORE_NAMESPACE::DObject;

class DObexSession;
class DObexAgent;
class DObexManagerPrivate;

class DObexManager : public QObject, public DObject
{
    Q_OBJECT
public:
    explicit DObexManager(QObject *parent = nullptr);
    ~DObexManager() override = default;

    Q_PROPERTY(bool available READ available CONSTANT)

    bool available() const;

public Q_SLOTS:
    DExpected<ObexSessionInfo> createSession(const QString &destination, const QVariantMap &args) const;
    DExpected<void> removeSession(const QSharedPointer<DObexSession> session) const;
    DExpected<void> registerAgent(const QSharedPointer<DObexAgent> &agent) const;
    DExpected<void> unregisterAgent(const QSharedPointer<DObexAgent> &agent) const;

    DExpected<QList<ObexSessionInfo>> sessions() const;
    DExpected<QSharedPointer<DObexSession>> sessionFromInfo(const ObexSessionInfo &info);

Q_SIGNALS:
    void sessionAdded(const ObexSessionInfo &info);
    void sessionRemoved(const ObexSessionInfo &info);

private:
    D_DECLARE_PRIVATE(DObexManager)
};

DBLUETOOTH_END_NAMESPACE
#endif
