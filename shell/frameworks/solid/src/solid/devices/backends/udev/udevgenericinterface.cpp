/*
    SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udevgenericinterface.h"

#include "udevdevice.h"

using namespace Solid::Backends::UDev;

GenericInterface::GenericInterface(UDevDevice *device)
    : DeviceInterface(device)
{
#if 0
    connect(device, SIGNAL(propertyChanged(QMap<QString,int>)),
            this, SIGNAL(propertyChanged(QMap<QString,int>)));
    connect(device, SIGNAL(conditionRaised(QString,QString)),
            this, SIGNAL(conditionRaised(QString,QString)));
#endif
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
    return m_device->propertyExists(key);
}

#include "moc_udevgenericinterface.cpp"
