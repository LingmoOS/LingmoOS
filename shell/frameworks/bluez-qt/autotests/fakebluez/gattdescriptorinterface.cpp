/*
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattdescriptorinterface.h"
#include "inputinterface.h"
#include "objectmanager.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>

// GattServiceObject
GattDescriptorObject::GattDescriptorObject(const QDBusObjectPath &path, QObject *parent)
    : QObject(parent)
{
    QDBusConnection::sessionBus().registerObject(path.path(), this);
}

// GattServiceInterface
GattDescriptorInterface::GattDescriptorInterface(const QDBusObjectPath &path, const QVariantMap &properties, QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setPath(path);
    setObjectParent(parent);
    setProperties(properties);
    setName(QStringLiteral("org.bluez.GattDescriptor1"));
}

QString GattDescriptorInterface::UUID() const
{
    return Object::property(QStringLiteral("UUID")).toString();
}

QDBusObjectPath GattDescriptorInterface::characteristic() const
{
    return Object::property(QStringLiteral("Characteristic")).value<QDBusObjectPath>();
}

QByteArray GattDescriptorInterface::value() const
{
    return Object::property(QStringLiteral("Value")).value<QByteArray>();
}

QStringList GattDescriptorInterface::flags() const
{
    return Object::property(QStringLiteral("Flags")).value<QStringList>();
}

quint16 GattDescriptorInterface::handle() const
{
    return Object::property(QStringLiteral("Handle")).value<quint16>();
}

void GattDescriptorInterface::setHandle(const quint16 handle)
{
    Object::changeProperty(QStringLiteral("Handle"), QVariant::fromValue(handle));
}

QByteArray GattDescriptorInterface::ReadValue(const QVariantMap& options)
{
    Q_UNUSED(options)
    QByteArray value = QByteArray("TEST");
    Object::changeProperty(QStringLiteral("Value"), value);
    return value;
}

void GattDescriptorInterface::WriteValue(const QByteArray& value, const QVariantMap& options)
{
    Q_UNUSED(options)
    Object::changeProperty(QStringLiteral("Value"), value);
}

#include "moc_gattdescriptorinterface.cpp"
