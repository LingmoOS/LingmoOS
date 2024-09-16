// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBLUETOOTHOBEXSESSION_H
#define DBLUETOOTHOBEXSESSION_H

#include "dbluetoothtypes.h"
#include <DExpected>
#include <DObject>
#include <QFileInfo>
#include <QDir>
#include <QBluetoothUuid>

DBLUETOOTH_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DExpected;
using DTK_CORE_NAMESPACE::DObject;

class DObexTransfer;
class DObexSessionPrivate;

class DObexSession : public QObject, public DObject
{
    Q_OBJECT
    explicit DObexSession(const ObexSessionInfo &info, QObject *parent = nullptr);
    friend class DObexManager;

public:
    ~DObexSession() override = default;

    Q_PROPERTY(QString source READ source CONSTANT)
    Q_PROPERTY(QString destination READ destination CONSTANT)
    Q_PROPERTY(QBluetoothUuid target READ target CONSTANT)
    Q_PROPERTY(QDir root READ root CONSTANT)
    Q_PROPERTY(ObexSessionInfo currentSession READ currentSession CONSTANT)

    QString source() const;
    QString destination() const;
    QBluetoothUuid target() const;
    QDir root() const;
    ObexSessionInfo currentSession() const;

    DExpected<QList<quint64>> transfers() const;
    DExpected<QSharedPointer<DObexTransfer>> transferFromId(quint64 transferId) const;

public Q_SLOTS:
    DExpected<QString> capabilities();
    DExpected<quint64> sendFile(const QFileInfo &file) const;

Q_SIGNALS:
    void transferAdded(quint64 transferId);
    void transferRemoved(quint64 transferId);

    void removed();

private:
    D_DECLARE_PRIVATE(DObexSession)
};

DBLUETOOTH_END_NAMESPACE
#endif
