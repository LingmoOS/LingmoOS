// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SERVICEMANAGERPUBLIC_H
#define SERVICEMANAGERPUBLIC_H

#include "qdbusservice.h"
#include "utils.h"

class ServiceManagerPublic : public QDBusService
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", ServiceManagerInterface);
    Q_PROPERTY(QStringList Groups READ groups);
    Q_PROPERTY(QString Version READ version);

public:
    explicit ServiceManagerPublic(QObject *parent = nullptr);
    ~ServiceManagerPublic();

    void addGroup(const QString &groupName);
    void init(const QDBusConnection::BusType &type);

    QStringList groups() const;

private:
    QString version() const;

private:
    QStringList m_groups;
};

#endif // PLUGINMANAGERPUBLIC_H
