/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2022 Pontus Sjögren
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattdescriptoradaptor.h"
#include "gattdescriptor.h"

namespace BluezQt
{

GattDescriptorAdaptor::GattDescriptorAdaptor(GattDescriptor *parent)
    : QDBusAbstractAdaptor(parent)
    , m_gattDescriptor(parent)
{
}

QString GattDescriptorAdaptor::uuid() const
{
    return m_gattDescriptor->uuid();
}

QDBusObjectPath GattDescriptorAdaptor::characteristic() const
{
    return m_gattDescriptor->characteristic();
}

QByteArray GattDescriptorAdaptor::value() const
{
    return m_gattDescriptor->readValue();
}

QStringList GattDescriptorAdaptor::flags() const
{
    return m_gattDescriptor->flags();
}

QByteArray GattDescriptorAdaptor::ReadValue(const QVariantMap &options)
{
    Q_UNUSED(options);
    return m_gattDescriptor->readValue();
}

void GattDescriptorAdaptor::WriteValue(QByteArray value)
{
    m_gattDescriptor->writeValue(value);
}

}

#include "moc_gattdescriptoradaptor.cpp"
