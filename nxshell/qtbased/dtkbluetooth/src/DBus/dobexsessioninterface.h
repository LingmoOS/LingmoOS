// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DOBEXSESSIONINTERFACE_H
#define DOBEXSESSIONINTERFACE_H

#include "dtkbluetooth_global.h"
#include <QObject>
#include <DObject>
#include <QDBusObjectPath>
#include <DDBusInterface>
#include <QDBusPendingReply>

DBLUETOOTH_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;

class DObexSessionInterface : public QObject
{
    Q_OBJECT
public:
    explicit DObexSessionInterface(const QString &path, QObject *parent = nullptr);
    ~DObexSessionInterface() override = default;

    Q_PROPERTY(QString source READ source CONSTANT)
    Q_PROPERTY(QString destination READ destination CONSTANT)
    Q_PROPERTY(QString target READ target CONSTANT)
    Q_PROPERTY(QString root READ root CONSTANT)

    QString source() const;
    QString destination() const;
    QString target() const;
    QString root() const;

    QString dBusPath() const;

public Q_SLOTS:
    QDBusPendingReply<QString> getCapabilities();

Q_SIGNALS:

    void removed();
    void transferAdded(quint64 transferId);
    void transferRemoved(quint64 transferId);

private:
    DDBusInterface *m_inter{nullptr};
};

DBLUETOOTH_END_NAMESPACE

#endif
