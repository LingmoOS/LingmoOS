/*
    SPDX-FileCopyrightText: 2009 Pino Toscano <pino@kde.org>
    SPDX-FileCopyrightText: 2012 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udisksgenericinterface.h"

#include "udisksdevice.h"

using namespace Solid::Backends::UDisks2;

GenericInterface::GenericInterface(Device *device)
    : DeviceInterface(device)
{
    connect(device, SIGNAL(propertyChanged(QMap<QString, int>)), this, SIGNAL(propertyChanged(QMap<QString, int>)));
}

GenericInterface::~GenericInterface()
{
}

QVariant GenericInterface::property(const QString &key) const
{
    return m_device->prop(key);
}

QVariantMap GenericInterface::allProperties() const
{
    return m_device->allProperties();
}

bool GenericInterface::propertyExists(const QString &key) const
{
    return m_device->propertyExists(key);
}

#include "moc_udisksgenericinterface.cpp"
