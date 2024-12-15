// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DOBEXCLIENTINTERFACE_H
#define DOBEXCLIENTINTERFACE_H

#include "dtkbluetooth_global.h"
#include <QDBusObjectPath>
#include <DDBusInterface>
#include <QDBusPendingReply>

DBLUETOOTH_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DDBusInterface;

class DObexClientInterface : public QObject
{
    Q_OBJECT
public:
    explicit DObexClientInterface(QObject *parent = nullptr);
    ~DObexClientInterface() override = default;

public Q_SLOTS:
    QDBusPendingReply<QDBusObjectPath> createSession(const QString &destination, const QVariantMap &args) const;
    QDBusPendingReply<void> removeSession(const QDBusObjectPath &path) const;

private:
    DDBusInterface *m_inter{nullptr};
};

DBLUETOOTH_END_NAMESPACE

#endif
