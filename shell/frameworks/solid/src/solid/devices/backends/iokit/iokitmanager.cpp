/*
    SPDX-FileCopyrightText: 2009 Harald Fernengel <harry@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "iokitmanager.h"
#include "iokitdevice.h"

#include <qdebug.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/usb/IOUSBLib.h>

#include <CoreFoundation/CoreFoundation.h>

namespace Solid
{
namespace Backends
{
namespace IOKit
{
class IOKitManagerPrivate
{
public:
    inline IOKitManagerPrivate()
        : port(nullptr)
        , source(nullptr)
    {
    }

    IONotificationPortRef port;
    CFRunLoopSourceRef source;

    static const char *typeToName(Solid::DeviceInterface::Type type);
    static QStringList devicesFromRegistry(io_iterator_t it);

    QSet<Solid::DeviceInterface::Type> supportedInterfaces;
};

// gets all registry paths from an iterator
QStringList IOKitManagerPrivate::devicesFromRegistry(io_iterator_t it)
{
    QStringList result;
    io_object_t obj;
    while ((obj = IOIteratorNext(it))) {
        CFStringRef pathRef = IORegistryEntryCopyPath(obj, kIOServicePlane);
        const QString path = QString::fromCFString(pathRef);
        CFRelease(pathRef);

        if (path.isEmpty()) {
            qWarning() << Q_FUNC_INFO << "IORegistryEntryCopyPath failed";
            continue;
        }
        result += path;
        const kern_return_t ret = IOObjectRelease(obj);
        if (ret != KERN_SUCCESS) {
            // very unlikely to happen - keep it a qDebug just in case.
            // compiler will nuke this code in release builds.
            qDebug() << Q_FUNC_INFO << "Unable to release object reference";
        }
    }
    IOObjectRelease(it);

    return result;
}

const char *IOKitManagerPrivate::typeToName(Solid::DeviceInterface::Type type)
{
    switch (type) {
    case Solid::DeviceInterface::Unknown:
        return 0;
    case Solid::DeviceInterface::Processor:
        return "AppleACPICPU";
    case Solid::DeviceInterface::Battery:
        return "AppleSmartBattery";

        // Solid::DeviceInterface::GenericInterface:
        // Solid::DeviceInterface::Block:
    case Solid::DeviceInterface::StorageAccess:
    case Solid::DeviceInterface::StorageDrive:
    case Solid::DeviceInterface::StorageVolume:
        return "IOMedia";
    case Solid::DeviceInterface::OpticalDrive:
    case Solid::DeviceInterface::OpticalDisc:
        return "IOCDMedia";
        // Solid::DeviceInterface::Camera:
        // Solid::DeviceInterface::PortableMediaPlayer:
    }

    return 0;
}

IOKitManager::IOKitManager(QObject *parent)
    : Solid::Ifaces::DeviceManager(parent)
    , d(new IOKitManagerPrivate)
{
    d->port = IONotificationPortCreate(kIOMasterPortDefault);
    if (!d->port) {
        qWarning() << Q_FUNC_INFO << "Unable to create notification port";
        return;
    }

    d->source = IONotificationPortGetRunLoopSource(d->port);
    if (!d->source) {
        qWarning() << Q_FUNC_INFO << "Unable to create notification source";
        return;
    }

    CFRunLoopAddSource(CFRunLoopGetCurrent(), d->source, kCFRunLoopDefaultMode);
    // clang-format off
    d->supportedInterfaces << Solid::DeviceInterface::GenericInterface
                           << Solid::DeviceInterface::Processor
                           << Solid::DeviceInterface::Block
                           << Solid::DeviceInterface::StorageAccess
                           << Solid::DeviceInterface::StorageDrive
                           << Solid::DeviceInterface::OpticalDrive
                           << Solid::DeviceInterface::StorageVolume
                           << Solid::DeviceInterface::OpticalDisc
                           << Solid::DeviceInterface::Camera
                           << Solid::DeviceInterface::PortableMediaPlayer
                           << Solid::DeviceInterface::Battery;
    // clang-format on
}

IOKitManager::~IOKitManager()
{
    if (d->source) {
        CFRunLoopRemoveSource(CFRunLoopGetCurrent(), d->source, kCFRunLoopDefaultMode);
    }
    if (d->port) {
        IONotificationPortDestroy(d->port);
    }

    delete d;
}

QString IOKitManager::udiPrefix() const
{
    return QString(); // FIXME: We should probably use a prefix there... has to be tested on Mac
}

QSet<Solid::DeviceInterface::Type> IOKitManager::supportedInterfaces() const
{
    return d->supportedInterfaces;
}

QStringList IOKitManager::allDevices()
{
    // use an IORegistry Iterator to iterate over all devices in the service plane

    io_iterator_t it;
    kern_return_t ret = IORegistryCreateIterator(kIOMasterPortDefault, kIOServicePlane, kIORegistryIterateRecursively, &it);
    if (ret != KERN_SUCCESS) {
        qWarning() << Q_FUNC_INFO << "unable to create iterator";
        return QStringList();
    }

    return IOKitManagerPrivate::devicesFromRegistry(it);
}

QStringList IOKitManager::devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type)
{
    QStringList result;

    if (type == Solid::DeviceInterface::Unknown) {
        // match all device interfaces
        result = allDevices();
    } else {
        const char *deviceClassName = IOKitManagerPrivate::typeToName(type);
        if (!deviceClassName) {
            return QStringList();
        }

        CFMutableDictionaryRef matchingDict = IOServiceMatching(deviceClassName);

        if (!matchingDict) {
            return QStringList();
        }

        io_iterator_t it = 0;

        // note - IOServiceGetMatchingServices dereferences the dict
        kern_return_t ret = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, &it);

        result = IOKitManagerPrivate::devicesFromRegistry(it);
    }

    // if the parentUdi is an empty string, return all matches
    if (parentUdi.isEmpty()) {
        return result;
    }

    // return only matches that start with the parent's UDI
    QStringList filtered;
    for (const QString &udi : std::as_const(result)) {
        if (udi.startsWith(parentUdi)) {
            filtered += udi;
        }
    }

    return filtered;
}

QObject *IOKitManager::createDevice(const QString &udi)
{
    CFStringRef path = udi.toCFString();
    io_registry_entry_t entry = IORegistryEntryCopyFromPath(kIOMasterPortDefault, path);
    CFRelease(path);

    // we have to do IOObjectConformsTo - comparing the class names is not good enough
    // if (IOObjectConformsTo(entry, kIOEthernetInterfaceClass)) {
    //}

    if (entry == MACH_PORT_NULL) {
        return 0;
    }

    return new IOKitDevice(udi, entry);
}

}
}
} // namespaces

#include "moc_iokitmanager.cpp"
