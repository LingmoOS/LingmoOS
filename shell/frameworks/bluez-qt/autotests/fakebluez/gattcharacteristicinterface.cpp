/*
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattcharacteristicinterface.h"
#include "inputinterface.h"
#include "objectmanager.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>

// GattServiceObject
GattCharacteristicObject::GattCharacteristicObject(const QDBusObjectPath &path, QObject *parent)
    : QObject(parent)
{
    QDBusConnection::sessionBus().registerObject(path.path(), this);
}

// GattServiceInterface
GattCharacteristicInterface::GattCharacteristicInterface(const QDBusObjectPath &path, const QVariantMap &properties, QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setPath(path);
    setObjectParent(parent);
    setProperties(properties);
    setName(QStringLiteral("org.bluez.GattCharacteristic1"));
}

QString GattCharacteristicInterface::UUID() const
{
    return Object::property(QStringLiteral("UUID")).toString();
}

QDBusObjectPath GattCharacteristicInterface::service() const
{
    return Object::property(QStringLiteral("Service")).value<QDBusObjectPath>();
}

QByteArray GattCharacteristicInterface::value() const
{
    return Object::property(QStringLiteral("Value")).value<QByteArray>();
}

bool GattCharacteristicInterface::notifying() const
{
    return Object::property(QStringLiteral("Notifying")).toBool();
}

QStringList GattCharacteristicInterface::flags() const
{
    return Object::property(QStringLiteral("Flags")).value<QStringList>();
}

quint16 GattCharacteristicInterface::handle() const
{
    return Object::property(QStringLiteral("Handle")).value<quint16>();
}

void GattCharacteristicInterface::setHandle(const quint16 handle)
{
    Object::changeProperty(QStringLiteral("Handle"), QVariant::fromValue(handle));
}

quint16 GattCharacteristicInterface::MTU() const
{
    return Object::property(QStringLiteral("MTU")).value<quint16>();
}

QByteArray GattCharacteristicInterface::ReadValue(const QVariantMap& options)
{
    Q_UNUSED(options)
    QByteArray value = QByteArray("TEST");
    Object::changeProperty(QStringLiteral("Value"), value);
    return value;
}

void GattCharacteristicInterface::WriteValue(const QByteArray& value, const QVariantMap& options)
{
    Q_UNUSED(options)
    Object::changeProperty(QStringLiteral("Value"), value);
}

void GattCharacteristicInterface::StartNotify()
{
    Object::changeProperty(QStringLiteral("Notifying"), true);
}

void GattCharacteristicInterface::StopNotify()
{
    Object::changeProperty(QStringLiteral("Notifying"), false);
}

#include "moc_gattcharacteristicinterface.cpp"
