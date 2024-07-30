/*
    SPDX-FileCopyrightText: 2005-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "device.h"
#include "device_p.h"
#include "devicemanager_p.h"
#include "devicenotifier.h"

#include "deviceinterface_p.h"
#include "soliddefs_p.h"

#include <solid/devices/ifaces/device.h>

#include <solid/battery.h>
#include <solid/block.h>
#include <solid/camera.h>
#include <solid/devices/ifaces/battery.h>
#include <solid/devices/ifaces/block.h>
#include <solid/devices/ifaces/camera.h>
#include <solid/devices/ifaces/genericinterface.h>
#include <solid/devices/ifaces/networkshare.h>
#include <solid/devices/ifaces/opticaldisc.h>
#include <solid/devices/ifaces/opticaldrive.h>
#include <solid/devices/ifaces/portablemediaplayer.h>
#include <solid/devices/ifaces/processor.h>
#include <solid/devices/ifaces/storageaccess.h>
#include <solid/devices/ifaces/storagedrive.h>
#include <solid/devices/ifaces/storagevolume.h>
#include <solid/genericinterface.h>
#include <solid/networkshare.h>
#include <solid/opticaldisc.h>
#include <solid/opticaldrive.h>
#include <solid/portablemediaplayer.h>
#include <solid/processor.h>
#include <solid/storageaccess.h>
#include <solid/storagedrive.h>
#include <solid/storagevolume.h>

Solid::Device::Device(const QString &udi)
{
    DeviceManagerPrivate *manager = static_cast<DeviceManagerPrivate *>(Solid::DeviceNotifier::instance());
    d = manager->findRegisteredDevice(udi);
}

Solid::Device::Device(const Device &device)
    : d(device.d)
{
}

Solid::Device::~Device()
{
}

Solid::Device &Solid::Device::operator=(const Solid::Device &device)
{
    d = device.d;
    return *this;
}

bool Solid::Device::isValid() const
{
    return d->backendObject() != nullptr;
}

QString Solid::Device::udi() const
{
    return d->udi();
}

QString Solid::Device::parentUdi() const
{
    return_SOLID_CALL(Ifaces::Device *, d->backendObject(), QString(), parentUdi());
}

Solid::Device Solid::Device::parent() const
{
    QString udi = parentUdi();

    if (udi.isEmpty()) {
        return Device();
    } else {
        return Device(udi);
    }
}

QString Solid::Device::vendor() const
{
    return_SOLID_CALL(Ifaces::Device *, d->backendObject(), QString(), vendor());
}

QString Solid::Device::product() const
{
    return_SOLID_CALL(Ifaces::Device *, d->backendObject(), QString(), product());
}

QString Solid::Device::icon() const
{
    return_SOLID_CALL(Ifaces::Device *, d->backendObject(), QString(), icon());
}

QStringList Solid::Device::emblems() const
{
    return_SOLID_CALL(Ifaces::Device *, d->backendObject(), QStringList(), emblems());
}

QString Solid::Device::displayName() const
{
    return_SOLID_CALL(Ifaces::Device *, d->backendObject(), QString(), displayName());
}

QString Solid::Device::description() const
{
    return_SOLID_CALL(Ifaces::Device *, d->backendObject(), QString(), description());
}

bool Solid::Device::isDeviceInterface(const DeviceInterface::Type &type) const
{
    return_SOLID_CALL(Ifaces::Device *, d->backendObject(), false, queryDeviceInterface(type));
}

#define deviceinterface_cast(IfaceType, DevType, backendObject) (qobject_cast<IfaceType *>(backendObject) ? new DevType(backendObject) : nullptr)

Solid::DeviceInterface *Solid::Device::asDeviceInterface(const DeviceInterface::Type &type)
{
    const Solid::DeviceInterface *interface = const_cast<const Device *>(this)->asDeviceInterface(type);
    return const_cast<Solid::DeviceInterface *>(interface);
}

const Solid::DeviceInterface *Solid::Device::asDeviceInterface(const DeviceInterface::Type &type) const
{
    Ifaces::Device *device = qobject_cast<Ifaces::Device *>(d->backendObject());

    if (device != nullptr) {
        DeviceInterface *iface = d->interface(type);

        if (iface != nullptr) {
            return iface;
        }

        QObject *dev_iface = device->createDeviceInterface(type);

        if (dev_iface != nullptr) {
            switch (type) {
            case DeviceInterface::GenericInterface:
                iface = deviceinterface_cast(Ifaces::GenericInterface, GenericInterface, dev_iface);
                break;
            case DeviceInterface::Processor:
                iface = deviceinterface_cast(Ifaces::Processor, Processor, dev_iface);
                break;
            case DeviceInterface::Block:
                iface = deviceinterface_cast(Ifaces::Block, Block, dev_iface);
                break;
            case DeviceInterface::StorageAccess:
                iface = deviceinterface_cast(Ifaces::StorageAccess, StorageAccess, dev_iface);
                break;
            case DeviceInterface::StorageDrive:
                iface = deviceinterface_cast(Ifaces::StorageDrive, StorageDrive, dev_iface);
                break;
            case DeviceInterface::OpticalDrive:
                iface = deviceinterface_cast(Ifaces::OpticalDrive, OpticalDrive, dev_iface);
                break;
            case DeviceInterface::StorageVolume:
                iface = deviceinterface_cast(Ifaces::StorageVolume, StorageVolume, dev_iface);
                break;
            case DeviceInterface::OpticalDisc:
                iface = deviceinterface_cast(Ifaces::OpticalDisc, OpticalDisc, dev_iface);
                break;
            case DeviceInterface::Camera:
                iface = deviceinterface_cast(Ifaces::Camera, Camera, dev_iface);
                break;
            case DeviceInterface::PortableMediaPlayer:
                iface = deviceinterface_cast(Ifaces::PortableMediaPlayer, PortableMediaPlayer, dev_iface);
                break;
            case DeviceInterface::Battery:
                iface = deviceinterface_cast(Ifaces::Battery, Battery, dev_iface);
                break;
            case DeviceInterface::NetworkShare:
                iface = deviceinterface_cast(Ifaces::NetworkShare, NetworkShare, dev_iface);
                break;
            case DeviceInterface::Unknown:
            case DeviceInterface::Last:
                break;
            }
        }

        if (iface != nullptr) {
            // Lie on the constness since we're simply doing caching here
            const_cast<Device *>(this)->d->setInterface(type, iface);
            iface->d_ptr->setDevicePrivate(d.data());
        }

        return iface;
    } else {
        return nullptr;
    }
}

//////////////////////////////////////////////////////////////////////

Solid::DevicePrivate::DevicePrivate(const QString &udi)
    : QObject()
    , QSharedData()
    , m_udi(udi)
{
}

Solid::DevicePrivate::~DevicePrivate()
{
    setBackendObject(nullptr);
}

void Solid::DevicePrivate::_k_destroyed(QObject *object)
{
    Q_UNUSED(object);
    setBackendObject(nullptr);
}

void Solid::DevicePrivate::setBackendObject(Ifaces::Device *object)
{
    if (m_backendObject) {
        m_backendObject.data()->disconnect(this);
    }

    delete m_backendObject.data();
    m_backendObject = object;

    if (object) {
        connect(object, SIGNAL(destroyed(QObject *)), this, SLOT(_k_destroyed(QObject *)));
    }

    if (!m_ifaces.isEmpty()) {
        qDeleteAll(m_ifaces);

        m_ifaces.clear();
        if (!ref.deref()) {
            deleteLater();
        }
    }
}

Solid::DeviceInterface *Solid::DevicePrivate::interface(const DeviceInterface::Type &type) const
{
    return m_ifaces[type];
}

void Solid::DevicePrivate::setInterface(const DeviceInterface::Type &type, DeviceInterface *interface)
{
    if (m_ifaces.isEmpty()) {
        ref.ref();
    }
    m_ifaces[type] = interface;
}

#include "moc_device_p.cpp"
