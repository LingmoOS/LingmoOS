// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef QDBUSSERVICE_P_H
#define QDBUSSERVICE_P_H

#include "service/serviceqtdbus.h"

#include <QObject>

class QDBusServicePrivate : public ServiceQtDBus
{
    Q_OBJECT
public:
    explicit QDBusServicePrivate(QObject *parent = nullptr);

    void initPolicy(QDBusConnection::BusType busType, QString policyFile);
    QDBusConnection qDbusConnection() const;
    void lockTimer(bool lock);

public Q_SLOTS:
    virtual void initService() override;
};

#endif // QDBUSSERVICEPRIVATE_H
