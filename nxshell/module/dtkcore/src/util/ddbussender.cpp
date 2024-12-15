// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ddbussender.h"

#include <QDBusInterface>
#include <QDebug>

DDBusSender::DDBusSender()
    : m_dbusData(std::make_shared<DDBusData>())
{
}

DDBusSender DDBusSender::service(const QString &service)
{
    m_dbusData->service = service;

    return *this;
}

DDBusSender DDBusSender::interface(const QString &interface)
{
    m_dbusData->interface = interface;

    return *this;
}

DDBusCaller DDBusSender::method(const QString &method)
{
    return DDBusCaller(method, m_dbusData);
}

DDBusProperty DDBusSender::property(const QString &property)
{
    return DDBusProperty(property, m_dbusData);
}

DDBusSender DDBusSender::system()
{
    DDBusSender self;
    self.type(QDBusConnection::SystemBus);
    return self;
}

DDBusSender DDBusSender::path(const QString &path)
{
    m_dbusData->path = path;

    return *this;
}

DDBusSender DDBusSender::type(const QDBusConnection::BusType busType)
{
    switch (busType)
    {
    case QDBusConnection::SessionBus:
        m_dbusData->connection = QDBusConnection::sessionBus();
        break;

    case QDBusConnection::SystemBus:
        m_dbusData->connection = QDBusConnection::systemBus();
        break;

    default:
        Q_UNREACHABLE_IMPL();
    }

    return *this;
}

DDBusData::DDBusData()
    : connection(QDBusConnection::sessionBus())
{

}

QDBusPendingCall DDBusData::asyncCallWithArguments(const QString &method, const QVariantList &arguments, const QString &iface)
{
    // When creating a QDBusAbstractInterface, Qt will try to invoke introspection into this dbus path;
    // This is costing in some cases when introspection is not ready;
    // Cause this is an asynchronous method, it'd be better not to wait for anything, just leave this to caller;
    // Use QDBusMessage to invoke directly instead of creating a QDBusInterface.
    const QString calledInterface = iface.isEmpty() ? interface : iface;
    QDBusMessage methodCall = QDBusMessage::createMethodCall(service, path, calledInterface, method);
    methodCall.setArguments(arguments);
    return connection.asyncCall(methodCall);
}

QDBusPendingCall DDBusCaller::call()
{
    return m_dbusData->asyncCallWithArguments(m_methodName, m_arguments);
}

DDBusCaller::DDBusCaller(const QString &method, std::shared_ptr<DDBusData> data)
    : m_dbusData(data)
    , m_methodName(method)
{
}

QDBusPendingCall DDBusProperty::get()
{
    QVariantList args{QVariant::fromValue(m_dbusData->interface), QVariant::fromValue(m_propertyName)};
    return m_dbusData->asyncCallWithArguments(QStringLiteral("Get"), args, QStringLiteral("org.freedesktop.DBus.Properties"));
}

DDBusProperty::DDBusProperty(const QString &property, std::shared_ptr<DDBusData> data)
    : m_dbusData(data)
    , m_propertyName(property)
{
}
