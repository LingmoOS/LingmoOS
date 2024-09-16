// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBLUETOOTHMANAGER_H
#define DBLUETOOTHMANAGER_H

#include <QSharedPointer>
#include <DExpected>
#include <DObject>
#include "dbluetoothadapter.h"

DBLUETOOTH_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DExpected;
using DTK_CORE_NAMESPACE::DObject;

class DAgent;
class DManagerPrivate;

class DManager : public QObject, public DObject
{
    Q_OBJECT
public:
    explicit DManager(QObject *parent = nullptr);
    ~DManager() override = default;

    Q_PROPERTY(bool available READ available CONSTANT)

    bool available() const;
    DExpected<QList<quint64>> adapters() const;
    DExpected<QSharedPointer<DAdapter>> adapterFromId(quint64 adapterId) const;
public Q_SLOTS:

    DExpected<void> registerAgent(const QSharedPointer<DAgent> &agent);
    DExpected<void> unregisterAgent(const QSharedPointer<DAgent> &agent);
    DExpected<void> requestDefaultAgent(const QSharedPointer<DAgent> &agent);

Q_SIGNALS:
    void adapterAdded(quint64 adapterId);
    void adapterRemoved(quint64 adapterId);

private:
    D_DECLARE_PRIVATE(DManager)
};

DBLUETOOTH_END_NAMESPACE
#endif
