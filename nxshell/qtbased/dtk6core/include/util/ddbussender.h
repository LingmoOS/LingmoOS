// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DDBUSSENDER_H
#define DDBUSSENDER_H

#include "dtkcore_global.h"

#include <QObject>
#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDBusInterface>

#include <memory>

class LIBDTKCORESHARED_EXPORT DDBusData
{
public:
    DDBusData();
    QDBusPendingCall asyncCallWithArguments(const QString &method, const QVariantList &arguments, const QString &iface = QString());

    QString service;
    QString path;
    QString interface;
    QString queryName;
    QDBusConnection connection;
};

class LIBDTKCORESHARED_EXPORT DDBusCaller
{
    friend class DDBusSender;

public:
    QDBusPendingCall call();

    template <typename T>
    DDBusCaller arg(const T &argument);

private:
    explicit DDBusCaller(const QString &method, std::shared_ptr<DDBusData> data);

private:
    std::shared_ptr<DDBusData> m_dbusData;
    QString m_methodName;
    QVariantList m_arguments;
};

template <typename T>
DDBusCaller DDBusCaller::arg(const T &argument)
{
    m_arguments << QVariant::fromValue(argument);

    return *this;
}

class LIBDTKCORESHARED_EXPORT DDBusProperty
{
    friend class DDBusSender;

public:
    QDBusPendingCall get();
    template <typename T>
    QDBusPendingCall set(const T &value);

private:
    explicit DDBusProperty(const QString &property, std::shared_ptr<DDBusData> data);

private:
    std::shared_ptr<DDBusData> m_dbusData;
    QString m_propertyName;
};

template <typename T>
QDBusPendingCall DDBusProperty::set(const T &value)
{
    QVariantList args{QVariant::fromValue(m_dbusData->interface), QVariant::fromValue(m_propertyName), QVariant::fromValue(QDBusVariant(value))};
    return m_dbusData->asyncCallWithArguments(QStringLiteral("Set"), args, QStringLiteral("org.freedesktop.DBus.Properties"));
}

class LIBDTKCORESHARED_EXPORT DDBusSender
{
public:
    explicit DDBusSender();

    DDBusSender service(const QString &service);
    DDBusSender interface(const QString &interface);
    DDBusSender path(const QString &path);
    DDBusCaller method(const QString &method);
    DDBusProperty property(const QString &property);

    static DDBusSender system();
private:
    DDBusSender type(const QDBusConnection::BusType busType);

private:
    std::shared_ptr<DDBusData> m_dbusData;
};

#endif // DDBUSSENDER_H
