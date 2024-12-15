// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SERVICEMANAGERPRIVATE_H
#define SERVICEMANAGERPRIVATE_H

#include "qdbusservice.h"
#include "utils.h"

class ServiceManagerPrivate : public QDBusService
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", ServiceManagerInterface);

public:
    explicit ServiceManagerPrivate(QObject *parent = nullptr);
    ~ServiceManagerPrivate();

    void init(const QDBusConnection::BusType &type);

public slots:
    void RegisterGroup(const QString &groupName, const QString &serviceName);

signals:
    void requestRegisterGroup(const QString &groupName, const QString &serviceName);
};

#endif // PLUGINMANAGERPRIVATE_H
