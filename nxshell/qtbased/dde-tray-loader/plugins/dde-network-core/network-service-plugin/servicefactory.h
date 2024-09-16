// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SERVICEFACTORY_H
#define SERVICEFACTORY_H

#include <QObject>

class SystemContainer;
class QDBusConnection;

class ServiceFactory : public QObject
{
    Q_OBJECT

public:
    explicit ServiceFactory(bool isSystem, QObject *parent = nullptr);
    QObject *serviceObject() const;

private:
    QObject *createServiceObject(bool isSystem);

private:
    QObject *m_serviceObject;
};

#endif // SERVICE_H
