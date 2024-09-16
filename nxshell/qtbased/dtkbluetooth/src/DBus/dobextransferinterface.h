// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DOBEXTRANSFERINTERFACE_H
#define DOBEXTRANSFERINTERFACE_H

#include "dtkbluetooth_global.h"
#include "dbluetoothdbustypes.h"
#include <QObject>
#include <DObject>
#include <QDBusObjectPath>
#include <DDBusInterface>
#include <QDBusPendingReply>

DBLUETOOTH_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;

class DObexTransferInterface : public QObject
{
    Q_OBJECT
public:
    explicit DObexTransferInterface(const QString &path, QObject *parent = nullptr);
    ~DObexTransferInterface() override = default;

    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QDBusObjectPath session READ session CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(quint64 size READ size CONSTANT)
    Q_PROPERTY(quint64 transferred READ transferred NOTIFY transferredChanged)
    Q_PROPERTY(QString filename READ filename CONSTANT)
    Q_PROPERTY(QString type READ type CONSTANT)

    QString status() const;
    QDBusObjectPath session() const;
    QString type() const;
    QString name() const;
    quint64 size() const;
    quint64 transferred() const;
    QString filename() const;

public Q_SLOTS:
    QDBusPendingReply<void> cancel() const;
    QDBusPendingReply<void> suspend() const;
    QDBusPendingReply<void> resume() const;

Q_SIGNALS:
    void statusChanged(const QString &status);
    void transferredChanged(quint64 transferred);

    void removed();

private:
    DDBusInterface *m_inter{nullptr};
};

DBLUETOOTH_END_NAMESPACE

#endif
