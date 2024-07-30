/*
    SPDX-FileCopyrightText: 2010 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udevdevice.h"

#include "cpuinfo.h"
#include "udevblock.h"
#include "udevcamera.h"
#include "udevgenericinterface.h"
#include "udevportablemediaplayer.h"
#include "udevprocessor.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <linux/if_arp.h>

using namespace Solid::Backends::UDev;

UDevDevice::UDevDevice(const UdevQt::Device device)
    : Solid::Ifaces::Device()
    , m_device(device)
{
}

UDevDevice::~UDevDevice()
{
}

QString UDevDevice::udi() const
{
    return devicePath();
}

QString UDevDevice::parentUdi() const
{
    return UDEV_UDI_PREFIX;
}

QString UDevDevice::vendor() const
{
    QString vendor = m_device.sysfsProperty("manufacturer").toString();
    if (vendor.isEmpty()) {
        if (queryDeviceInterface(Solid::DeviceInterface::Processor)) {
            // sysfs doesn't have anything useful here
            vendor = extractCpuVendor(deviceNumber());
        }

        if (vendor.isEmpty()) {
            vendor = m_device.deviceProperty("ID_VENDOR").toString().replace('_', ' ');
        }
    }
    return vendor;
}

QString UDevDevice::product() const
{
    QString product = m_device.sysfsProperty("product").toString();
    if (product.isEmpty()) {
        if (queryDeviceInterface(Solid::DeviceInterface::Processor)) {
            // sysfs doesn't have anything useful here
            product = extractCpuModel(deviceNumber());
        }

        if (product.isEmpty()) {
            product = m_device.deviceProperty("ID_MODEL").toString().replace('_', ' ');
        }
    }
    return product;
}

QString UDevDevice::icon() const
{
    if (parentUdi().isEmpty()) {
        return QLatin1String("computer");
    }

    if (queryDeviceInterface(Solid::DeviceInterface::Processor)) {
        return QLatin1String("cpu");
    } else if (queryDeviceInterface(Solid::DeviceInterface::PortableMediaPlayer)) {
        // TODO: check out special cases like iPod
        return QLatin1String("multimedia-player");
    } else if (queryDeviceInterface(Solid::DeviceInterface::Camera)) {
        return QLatin1String("camera-photo");
    }

    return QString();
}

QStringList UDevDevice::emblems() const
{
    return QStringList();
}

QString UDevDevice::description() const
{
    if (parentUdi().isEmpty()) {
        return tr("Computer");
    }

    if (queryDeviceInterface(Solid::DeviceInterface::Processor)) {
        return tr("Processor");
    } else if (queryDeviceInterface(Solid::DeviceInterface::PortableMediaPlayer)) {
        /*
         * HACK: As Media player is very generic return the device product instead
         *       until we can return the Name.
         */
        const PortableMediaPlayer *player = new PortableMediaPlayer(const_cast<UDevDevice *>(this));
        if (player->supportedProtocols().contains("mtp")) {
            return product();
        } else {
            // TODO: check out special cases like iPod
            return tr("Portable Media Player");
        }
    } else if (queryDeviceInterface(Solid::DeviceInterface::Camera)) {
        return tr("Camera");
    }

    return QString();
}

bool UDevDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    switch (type) {
    case Solid::DeviceInterface::GenericInterface:
        return true;

    case Solid::DeviceInterface::Processor:
        return m_device.subsystem() == QLatin1String("cpu");

    case Solid::DeviceInterface::Camera:
        return m_device.subsystem() == QLatin1String("usb") && property("ID_GPHOTO2").isValid();

    case Solid::DeviceInterface::PortableMediaPlayer:
        return m_device.subsystem() == QLatin1String("usb") && property("ID_MEDIA_PLAYER").isValid();

    case Solid::DeviceInterface::Block:
        return !property("MAJOR").toString().isEmpty();

    default:
        return false;
    }
}

QObject *UDevDevice::createDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    if (!queryDeviceInterface(type)) {
        return nullptr;
    }

    switch (type) {
    case Solid::DeviceInterface::GenericInterface:
        return new GenericInterface(this);

    case Solid::DeviceInterface::Processor:
        return new Processor(this);

    case Solid::DeviceInterface::Camera:
        return new Camera(this);

    case Solid::DeviceInterface::PortableMediaPlayer:
        return new PortableMediaPlayer(this);

    case Solid::DeviceInterface::Block:
        return new Block(this);

    default:
        qFatal("Shouldn't happen");
        return nullptr;
    }
}

QString UDevDevice::device() const
{
    return devicePath();
}

QVariant UDevDevice::property(const QString &key) const
{
    const QVariant res = m_device.deviceProperty(key);
    if (res.isValid()) {
        return res;
    }
    return m_device.sysfsProperty(key);
}

QMap<QString, QVariant> UDevDevice::allProperties() const
{
    QMap<QString, QVariant> res;
    const QStringList properties = m_device.deviceProperties();
    for (const QString &prop : properties) {
        res[prop] = property(prop);
    }
    return res;
}

bool UDevDevice::propertyExists(const QString &key) const
{
    return m_device.deviceProperties().contains(key);
}

QString UDevDevice::systemAttribute(const char *attribute) const
{
    return m_device.sysfsProperty(attribute).toString();
}

QString UDevDevice::deviceName() const
{
    return m_device.sysfsPath();
}

int UDevDevice::deviceNumber() const
{
    return m_device.sysfsNumber();
}

QString UDevDevice::devicePath() const
{
    return QString(UDEV_UDI_PREFIX) + deviceName();
}

UdevQt::Device UDevDevice::udevDevice()
{
    return m_device;
}

#include "moc_udevdevice.cpp"
