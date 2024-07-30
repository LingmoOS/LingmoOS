/*
    SPDX-FileCopyrightText: 2009 Harald Fernengel <harry@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "iokitgenericinterface.h"

#include "iokitdevice.h"

using namespace Solid::Backends::IOKit;

GenericInterface::GenericInterface(IOKitDevice *device)
    : DeviceInterface(device)
{
}

GenericInterface::~GenericInterface()
{
}

QVariant GenericInterface::property(const QString &key) const
{
    return m_device->property(key);
}

QMap<QString, QVariant> GenericInterface::allProperties() const
{
    return m_device->allProperties();
}

bool GenericInterface::propertyExists(const QString &key) const
{
    return m_device->iOKitPropertyExists(key);
}

#include "moc_iokitgenericinterface.cpp"
