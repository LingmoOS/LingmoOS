// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBLUETOOTHOBEXTRANSFER_H
#define DBLUETOOTHOBEXTRANSFER_H

#include "dbluetoothtypes.h"
#include <DExpected>
#include <QFileInfo>
#include <DObject>

DBLUETOOTH_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DExpected;
using DTK_CORE_NAMESPACE::DObject;

class DObexTransferPrivate;

class DObexTransfer : public QObject, public DObject
{
    Q_OBJECT
    explicit DObexTransfer(const ObexSessionInfo &info, quint64 transferId, QObject *parent = nullptr);
    friend class DObexSession;

public:
    ~DObexTransfer() override;

    enum TransferStatus { Unknown, Queued, Active, Suspended, Complete, Error };
    Q_ENUM(TransferStatus)

    Q_PROPERTY(TransferStatus status READ status NOTIFY statusChanged)
    Q_PROPERTY(ObexSessionInfo session READ session CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(quint64 size READ size CONSTANT)
    Q_PROPERTY(QString type READ type CONSTANT)
    Q_PROPERTY(quint64 transferred READ transferred NOTIFY transferredChanged)
    Q_PROPERTY(QFileInfo filename READ filename CONSTANT)

    TransferStatus status() const;
    ObexSessionInfo session() const;
    QString name() const;
    QString type() const;
    quint64 size() const;
    quint64 transferred() const;
    QFileInfo filename() const;

public Q_SLOTS:
    DExpected<void> cancel() const;

Q_SIGNALS:
    void statusChanged(TransferStatus status);
    void transferredChanged(quint64 transferred);

    void removed();

private:
    D_DECLARE_PRIVATE(DObexTransfer);
};

DBLUETOOTH_END_NAMESPACE
#endif
