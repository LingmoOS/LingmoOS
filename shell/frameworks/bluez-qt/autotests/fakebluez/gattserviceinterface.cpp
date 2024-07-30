/*
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattserviceinterface.h"
#include "inputinterface.h"
#include "objectmanager.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>

// GattServiceObject
GattServiceObject::GattServiceObject(const QDBusObjectPath &path, QObject *parent)
    : QObject(parent)
{
    QDBusConnection::sessionBus().registerObject(path.path(), this);
}

// GattServiceInterface
GattServiceInterface::GattServiceInterface(const QDBusObjectPath &path, const QVariantMap &properties, QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setPath(path);
    setObjectParent(parent);
    setProperties(properties);
    setName(QStringLiteral("org.bluez.GattService1"));
}

QString GattServiceInterface::UUID() const
{
    return Object::property(QStringLiteral("UUID")).toString();
}

bool GattServiceInterface::primary() const
{
    return Object::property(QStringLiteral("Primary")).toBool();
}

QDBusObjectPath GattServiceInterface::device() const
{
    return Object::property(QStringLiteral("Device")).value<QDBusObjectPath>();
}

QList<QDBusObjectPath> GattServiceInterface::includes() const
{
    return Object::property(QStringLiteral("Includes")).value<QList<QDBusObjectPath>>();
}

quint16 GattServiceInterface::handle() const
{
    return Object::property(QStringLiteral("Handle")).value<quint16>();
}

void GattServiceInterface::setHandle(const quint16 handle)
{
    Object::changeProperty(QStringLiteral("Handle"), QVariant::fromValue(handle));
}

#include "moc_gattserviceinterface.cpp"
