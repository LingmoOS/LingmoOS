// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "qdbusservice.h"

#include "qdbusservice_p.h"

#include <QCoreApplication>

QDBusService::QDBusService(QObject *parent)
    : QObject(parent)
    , d_ptr(new QDBusServicePrivate(this))
{
}

void QDBusService::initPolicy(const QDBusConnection::BusType &busType, const QString &policyFile)
{
    Q_D(QDBusService);
    d->initPolicy(busType, policyFile);
    connect(d, &QDBusServicePrivate::idleSignal, [] {
        qApp->quit();
    });
}

QDBusConnection QDBusService::qDbusConnection() const
{
    Q_D(const QDBusService);
    return d->qDbusConnection();
}

void QDBusService::lockTimer(bool lock)
{
    Q_D(QDBusService);
    d->lockTimer(lock);
}
