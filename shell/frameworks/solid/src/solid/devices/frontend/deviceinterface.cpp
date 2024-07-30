/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "deviceinterface.h"
#include "deviceinterface_p.h"

#include <solid/devices/ifaces/deviceinterface.h>

#include <QMetaEnum>

Solid::DeviceInterface::DeviceInterface(DeviceInterfacePrivate &dd, QObject *backendObject)
    : d_ptr(&dd)
{
    Q_D(DeviceInterface);

    d->setBackendObject(backendObject);
}

Solid::DeviceInterface::~DeviceInterface()
{
    delete d_ptr->backendObject();
    delete d_ptr;
    d_ptr = nullptr;
}

bool Solid::DeviceInterface::isValid() const
{
    Q_D(const DeviceInterface);
    return d->backendObject() != nullptr;
}

QString Solid::DeviceInterface::typeToString(Type type)
{
    int index = staticMetaObject.indexOfEnumerator("Type");
    QMetaEnum metaEnum = staticMetaObject.enumerator(index);
    return QString(metaEnum.valueToKey((int)type));
}

Solid::DeviceInterface::Type Solid::DeviceInterface::stringToType(const QString &type)
{
    int index = staticMetaObject.indexOfEnumerator("Type");
    QMetaEnum metaEnum = staticMetaObject.enumerator(index);
    return (Type)metaEnum.keyToValue(type.toUtf8());
}

QString Solid::DeviceInterface::typeDescription(Type type)
{
    switch (type) {
    case Unknown:
        return tr("Unknown", "Unknown device type");
    case GenericInterface:
        return tr("Generic Interface", "Generic Interface device type");
    case Processor:
        return tr("Processor", "Processor device type");
    case Block:
        return tr("Block", "Block device type");
    case StorageAccess:
        return tr("Storage Access", "Storage Access device type");
    case StorageDrive:
        return tr("Storage Drive", "Storage Drive device type");
    case OpticalDrive:
        return tr("Optical Drive", "Optical Drive device type");
    case StorageVolume:
        return tr("Storage Volume", "Storage Volume device type");
    case OpticalDisc:
        return tr("Optical Disc", "Optical Disc device type");
    case Camera:
        return tr("Camera", "Camera device type");
    case PortableMediaPlayer:
        return tr("Portable Media Player", "Portable Media Player device type");
    case Battery:
        return tr("Battery", "Battery device type");
    case NetworkShare:
        return tr("Network Share", "Network Share device type");
    case Last:
        return QString();
    }
    return QString();
}

Solid::DeviceInterfacePrivate::DeviceInterfacePrivate()
    : m_devicePrivate(nullptr)
{
}

Solid::DeviceInterfacePrivate::~DeviceInterfacePrivate()
{
}

QObject *Solid::DeviceInterfacePrivate::backendObject() const
{
    return m_backendObject.data();
}

void Solid::DeviceInterfacePrivate::setBackendObject(QObject *object)
{
    m_backendObject = object;
}

Solid::DevicePrivate *Solid::DeviceInterfacePrivate::devicePrivate() const
{
    return m_devicePrivate;
}

void Solid::DeviceInterfacePrivate::setDevicePrivate(DevicePrivate *devicePrivate)
{
    m_devicePrivate = devicePrivate;
}

#include "moc_deviceinterface.cpp"
