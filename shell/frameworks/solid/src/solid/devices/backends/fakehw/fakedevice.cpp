/*
    SPDX-FileCopyrightText: 2006 Michaël Larouche <michael.larouche@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "fakedevice.h"
#include "fakedevice_p.h"

#include "fakebattery.h"
#include "fakeblock.h"
#include "fakecamera.h"
#include "fakecdrom.h"
#include "fakedeviceinterface.h"
#include "fakegenericinterface.h"
#include "fakenetworkshare.h"
#include "fakeopticaldisc.h"
#include "fakeportablemediaplayer.h"
#include "fakeprocessor.h"
#include "fakestorage.h"
#include "fakestorageaccess.h"
#include "fakevolume.h"

#include <QStringList>
#ifdef HAVE_DBUS
#include <QDBusConnection>
#endif

#include <solid/genericinterface.h>

using namespace Solid::Backends::Fake;

FakeDevice::FakeDevice(const QString &udi, const QMap<QString, QVariant> &propertyMap)
    : Solid::Ifaces::Device()
    , d(new Private)
{
    d->udi = udi;
    d->propertyMap = propertyMap;
    d->interfaceList = d->propertyMap["interfaces"].toString().simplified().split(',');
    d->interfaceList << "GenericInterface";
    d->locked = false;
    d->broken = false;

#ifdef HAVE_DBUS
    QDBusConnection::sessionBus().registerObject(udi, this, QDBusConnection::ExportNonScriptableSlots);
#endif

    // Force instantiation of all the device interfaces
    // this way they'll get exported on the bus
    // that means they'll be created twice, but that won't be
    // a problem for unit testing.
    for (const QString &interface : std::as_const(d->interfaceList)) {
        Solid::DeviceInterface::Type type = Solid::DeviceInterface::stringToType(interface);
        createDeviceInterface(type);
    }

    connect(d.data(), SIGNAL(propertyChanged(QMap<QString, int>)), this, SIGNAL(propertyChanged(QMap<QString, int>)));
    connect(d.data(), SIGNAL(conditionRaised(QString, QString)), this, SIGNAL(conditionRaised(QString, QString)));
}

FakeDevice::FakeDevice(const FakeDevice &dev)
    : Solid::Ifaces::Device()
    , d(dev.d)
{
    connect(d.data(), SIGNAL(propertyChanged(QMap<QString, int>)), this, SIGNAL(propertyChanged(QMap<QString, int>)));
    connect(d.data(), SIGNAL(conditionRaised(QString, QString)), this, SIGNAL(conditionRaised(QString, QString)));
}

FakeDevice::~FakeDevice()
{
#ifdef HAVE_DBUS
    QDBusConnection::sessionBus().unregisterObject(d->udi, QDBusConnection::UnregisterTree);
#endif
}

QString FakeDevice::udi() const
{
    return d->udi;
}

QString FakeDevice::parentUdi() const
{
    return d->propertyMap["parent"].toString();
}

QString FakeDevice::vendor() const
{
    return d->propertyMap["vendor"].toString();
}

QString FakeDevice::product() const
{
    return d->propertyMap["name"].toString();
}

QString FakeDevice::icon() const
{
    if (parentUdi().isEmpty()) {
        return "system";
    } else if (queryDeviceInterface(Solid::DeviceInterface::OpticalDrive)) {
        return "cdrom-unmount";
    } else if (queryDeviceInterface(Solid::DeviceInterface::PortableMediaPlayer)) {
        return "ipod-unmount";
    } else if (queryDeviceInterface(Solid::DeviceInterface::Camera)) {
        return "camera-unmount";
    } else if (queryDeviceInterface(Solid::DeviceInterface::Processor)) {
        return "cpu";
    } else if (queryDeviceInterface(Solid::DeviceInterface::StorageDrive)) {
        return "hdd-unmount";
    } else if (queryDeviceInterface(Solid::DeviceInterface::Block)) {
        return "blockdevice";
    } else {
        return "hwinfo";
    }
}

QStringList FakeDevice::emblems() const
{
    QStringList res;

    if (queryDeviceInterface(Solid::DeviceInterface::StorageAccess)) {
        if (property("isMounted").toBool()) {
            res << "emblem-mounted";
        } else {
            res << "emblem-unmounted";
        }
    }

    return res;
}

QString FakeDevice::description() const
{
    return product();
}

QVariant FakeDevice::property(const QString &key) const
{
    return d->propertyMap[key];
}

QMap<QString, QVariant> FakeDevice::allProperties() const
{
    return d->propertyMap;
}

bool FakeDevice::propertyExists(const QString &key) const
{
    return d->propertyMap.contains(key);
}

bool FakeDevice::setProperty(const QString &key, const QVariant &value)
{
    if (d->broken) {
        return false;
    }

    Solid::GenericInterface::PropertyChange change_type = Solid::GenericInterface::PropertyModified;

    if (!d->propertyMap.contains(key)) {
        change_type = Solid::GenericInterface::PropertyAdded;
    }

    d->propertyMap[key] = value;

    QMap<QString, int> change;
    change[key] = change_type;

    Q_EMIT d->propertyChanged(change);

    return true;
}

bool FakeDevice::removeProperty(const QString &key)
{
    if (d->broken || !d->propertyMap.contains(key)) {
        return false;
    }

    d->propertyMap.remove(key);

    QMap<QString, int> change;
    change[key] = Solid::GenericInterface::PropertyRemoved;

    Q_EMIT d->propertyChanged(change);

    return true;
}

void FakeDevice::setBroken(bool broken)
{
    d->broken = broken;
}

bool FakeDevice::isBroken()
{
    return d->broken;
}

bool FakeDevice::lock(const QString &reason)
{
    if (d->broken || d->locked) {
        return false;
    }

    d->locked = true;
    d->lockReason = reason;

    return true;
}

bool FakeDevice::unlock()
{
    if (d->broken || !d->locked) {
        return false;
    }

    d->locked = false;
    d->lockReason.clear();

    return true;
}

bool FakeDevice::isLocked() const
{
    return d->locked;
}

QString FakeDevice::lockReason() const
{
    return d->lockReason;
}

void FakeDevice::raiseCondition(const QString &condition, const QString &reason)
{
    Q_EMIT d->conditionRaised(condition, reason);
}

bool FakeDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    return d->interfaceList.contains(Solid::DeviceInterface::typeToString(type));
}

QObject *FakeDevice::createDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    // Do not try to cast with a unsupported device interface.
    if (!queryDeviceInterface(type)) {
        return nullptr;
    }

    FakeDeviceInterface *iface = nullptr;

    switch (type) {
    case Solid::DeviceInterface::GenericInterface:
        iface = new FakeGenericInterface(this);
        break;
    case Solid::DeviceInterface::Processor:
        iface = new FakeProcessor(this);
        break;
    case Solid::DeviceInterface::Block:
        iface = new FakeBlock(this);
        break;
    case Solid::DeviceInterface::StorageDrive:
        iface = new FakeStorage(this);
        break;
    case Solid::DeviceInterface::OpticalDrive:
        iface = new FakeCdrom(this);
        break;
    case Solid::DeviceInterface::StorageVolume:
        iface = new FakeVolume(this);
        break;
    case Solid::DeviceInterface::OpticalDisc:
        iface = new FakeOpticalDisc(this);
        break;
    case Solid::DeviceInterface::StorageAccess:
        iface = new FakeStorageAccess(this);
        break;
    case Solid::DeviceInterface::Camera:
        iface = new FakeCamera(this);
        break;
    case Solid::DeviceInterface::PortableMediaPlayer:
        iface = new FakePortableMediaPlayer(this);
        break;
    case Solid::DeviceInterface::Battery:
        iface = new FakeBattery(this);
        break;
    case Solid::DeviceInterface::NetworkShare:
        iface = new FakeNetworkShare(this);
        break;
    case Solid::DeviceInterface::Unknown:
        break;
    case Solid::DeviceInterface::Last:
        break;
    }

#ifdef HAVE_DBUS
    if (iface) {
        QDBusConnection::sessionBus().registerObject(d->udi + '/' + Solid::DeviceInterface::typeToString(type),
                                                     iface,
                                                     QDBusConnection::ExportNonScriptableSlots);
    }
#endif

    return iface;
}

#include "moc_fakedevice.cpp"
#include "moc_fakedevice_p.cpp"
