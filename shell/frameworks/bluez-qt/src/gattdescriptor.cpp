/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2022 Pontus Sjögren
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattdescriptor.h"
#include "gattcharacteristic.h"
#include "gattdescriptor_p.h"

namespace BluezQt
{

GattDescriptor *GattDescriptor::createUserDescription(QString const &description, GattCharacteristic *characteristic)
{
    return new GattDescriptor(QLatin1String("2901"), {QLatin1String("read")}, description.toUtf8(), characteristic);
}

GattDescriptor::GattDescriptor(const QString &uuid, GattCharacteristic *parent)
    : GattDescriptor(uuid, {}, {}, parent)
{
}

GattDescriptor::GattDescriptor(const QString &uuid, const QStringList &flags, GattCharacteristic *parent)
    : GattDescriptor(uuid, flags, {}, parent)
{
}

GattDescriptor::GattDescriptor(const QString &uuid, const QStringList &flags, const QByteArray &initialValue, GattCharacteristic *parent)
    : QObject(parent)
    , d(new GattDescriptorPrivate(uuid, flags, initialValue, parent))
{
}

GattDescriptor::~GattDescriptor() = default;

QByteArray GattDescriptor::readValue()
{
    return d->m_value;
}

void GattDescriptor::writeValue(const QByteArray &value)
{
    d->m_value = value;
}

QString GattDescriptor::uuid() const
{
    return d->m_uuid;
}

QDBusObjectPath GattDescriptor::characteristic() const
{
    return d->m_characteristic->objectPath();
}

QStringList GattDescriptor::flags() const
{
    return d->m_flags;
}

QDBusObjectPath GattDescriptor::objectPath() const
{
    return d->m_objectPath;
}

}

#include "moc_gattdescriptor.cpp"
