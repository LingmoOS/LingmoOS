/*
    SPDX-FileCopyrightText: 2009 Pino Toscano <pino@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "upowergenericinterface.h"

#include "upowerdevice.h"

using namespace Solid::Backends::UPower;

GenericInterface::GenericInterface(UPowerDevice *device)
    : DeviceInterface(device)
{
    connect(device, &UPowerDevice::propertyChanged,
            this, &GenericInterface::propertyChanged);
#if 0 // not used/implemented
    connect(device, &UPowerDevice::conditionRaised,
            this, &UPowerDevice::conditionRaised);
#endif
}

GenericInterface::~GenericInterface()
{
}

QVariant GenericInterface::property(const QString &key) const
{
    return m_device.data()->prop(key);
}

QMap<QString, QVariant> GenericInterface::allProperties() const
{
    return m_device.data()->allProperties();
}

bool GenericInterface::propertyExists(const QString &key) const
{
    return m_device.data()->propertyExists(key);
}

#include "moc_upowergenericinterface.cpp"
