/*
    SPDX-FileCopyrightText: 2009 Harald Fernengel <harry@kdevelop.org>
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "iokitdevice.h"
#include "iokitbattery.h"
#include "iokitgenericinterface.h"
#include "iokitopticaldisc.h"
#include "iokitopticaldrive.h"
#include "iokitprocessor.h"
#include "iokitstorage.h"
#include "iokitstorageaccess.h"
#include "iokitvolume.h"

#include <QDebug>
#include <QSet>
#include <QUrl>

#include <sys/sysctl.h>
#include <sys/types.h>

#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/usb/IOUSBLib.h>

#include <CoreFoundation/CoreFoundation.h>

// from cfhelper.cpp
extern QMap<QString, QVariant> q_toVariantMap(const CFMutableDictionaryRef &dict);
extern bool q_sysctlbyname(const char *name, QString &result);

typedef QSet<Solid::DeviceInterface::Type> DeviceInterfaceTypes;

namespace Solid
{
namespace Backends
{
namespace IOKit
{
// returns a solid type from an entry and its properties
static DeviceInterfaceTypes typesFromEntry(const io_registry_entry_t &entry, const QMap<QString, QVariant> &properties, Solid::DeviceInterface::Type &mainType)
{
    DeviceInterfaceTypes types;
    mainType = Solid::DeviceInterface::Unknown;
    if (IOObjectConformsTo(entry, "AppleACPICPU")) {
        mainType = Solid::DeviceInterface::Processor;
        types << mainType;
    }
    if (IOObjectConformsTo(entry, "AppleSmartBattery")) {
        mainType = Solid::DeviceInterface::Battery;
        types << mainType;
    }
    const QString bsdName = QStringLiteral("BSD Name");
    const QString leaf = QStringLiteral("Leaf");
    if (IOObjectConformsTo(entry, "IOCDMedia") //
        || IOObjectConformsTo(entry, "IODVDMedia") //
        || IOObjectConformsTo(entry, "IOBDMedia")) {
        mainType = Solid::DeviceInterface::OpticalDrive;
        types << mainType << Solid::DeviceInterface::OpticalDisc;
    }
    if (properties.contains(bsdName) && properties.value(bsdName).toString().startsWith(QStringLiteral("disk"))) {
        if ((properties.contains(leaf) && properties.value(leaf).toBool() == false) //
            || mainType == Solid::DeviceInterface::OpticalDrive) {
            if (mainType == Solid::DeviceInterface::Unknown) {
                mainType = Solid::DeviceInterface::StorageDrive;
            }
            types << Solid::DeviceInterface::StorageDrive;
        } else if (mainType == Solid::DeviceInterface::Unknown) {
            mainType = Solid::DeviceInterface::StorageVolume;
        }
        types << Solid::DeviceInterface::StorageVolume;
    }

    if (types.isEmpty()) {
        types << mainType;
        //         qWarning() << "unsupported entry" << entry << "with properties" << properties;
    }

    return types;
}

// gets all properties from an entry into a QMap
static QMap<QString, QVariant> getProperties(const io_registry_entry_t &entry)
{
    CFMutableDictionaryRef propertyDict = 0;

    if (IORegistryEntryCreateCFProperties(entry, &propertyDict, kCFAllocatorDefault, kNilOptions) != KERN_SUCCESS) {
        return QMap<QString, QVariant>();
    }

    QMap<QString, QVariant> result = q_toVariantMap(propertyDict);

    CFRelease(propertyDict);

    io_name_t className;
    IOObjectGetClass(entry, className);
    result["className"] = QString::fromUtf8(className);

    return result;
}

// gets the parent's Udi from an entry
static QString getParentDeviceUdi(const io_registry_entry_t &entry)
{
    io_registry_entry_t parent = 0;
    kern_return_t ret = IORegistryEntryGetParentEntry(entry, kIOServicePlane, &parent);
    if (ret != KERN_SUCCESS) {
        // don't release parent here - docs say only on success
        return QString();
    }

    CFStringRef path = IORegistryEntryCopyPath(parent, kIOServicePlane);
    QString result = QString::fromCFString(path);
    CFRelease(path);

    // now we can release the parent
    IOObjectRelease(parent);

    return result;
}

static const QString computerModel()
{
    QString qModel;
    q_sysctlbyname("hw.model", qModel);
    return qModel;
}

class IOKitDevicePrivate
{
public:
    inline IOKitDevicePrivate()
        : type({Solid::DeviceInterface::Unknown})
        , parentDevice(nullptr)
    {
    }
    ~IOKitDevicePrivate()
    {
        if (parentDevice) {
            delete parentDevice;
            parentDevice = nullptr;
        }
    }

    void init(const QString &udiString, const io_registry_entry_t &entry);
    IOKitDevice *getParentDevice();

    QString udi;
    QString parentUdi;
    QMap<QString, QVariant> properties;
    DeviceInterfaceTypes type;
    Solid::DeviceInterface::Type mainType;
    IOKitDevice *parentDevice;
};

void IOKitDevicePrivate::init(const QString &udiString, const io_registry_entry_t &entry)
{
    Q_ASSERT(entry != MACH_PORT_NULL);

    udi = udiString;

    properties = getProperties(entry);

    parentUdi = getParentDeviceUdi(entry);
    type = typesFromEntry(entry, properties, mainType);
    if (udi.contains(QStringLiteral("IOBD")) || udi.contains(QStringLiteral("BD PX"))) {
        qWarning() << "Solid: BlueRay entry" << entry << "mainType=" << mainType << "typeList:" << type << "with properties" << properties;
    }
    if (mainType != Solid::DeviceInterface::Unknown) { }

    IOObjectRelease(entry);
}

IOKitDevice *IOKitDevicePrivate::getParentDevice()
{
    if (!parentDevice) {
        parentDevice = new IOKitDevice(parentUdi);
    }
    return parentDevice;
}

IOKitDevice::IOKitDevice(const QString &udi, const io_registry_entry_t &entry)
    : d(new IOKitDevicePrivate)
{
    d->init(udi, entry);
}

IOKitDevice::IOKitDevice(const QString &udi)
    : d(new IOKitDevicePrivate)
{
    if (udi.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Tried to create Device from empty UDI";
        return;
    }

    CFStringRef path = udi.toCFString();
    io_registry_entry_t entry = IORegistryEntryCopyFromPath(kIOMasterPortDefault, path);
    CFRelease(path);

    if (entry == MACH_PORT_NULL) {
        qWarning() << Q_FUNC_INFO << "Tried to create Device from invalid UDI" << udi;
        return;
    }

    d->init(udi, entry);
}

IOKitDevice::IOKitDevice(const IOKitDevice &device)
    : d(new IOKitDevicePrivate)
{
    if (device.udi().isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Tried to create Device from empty UDI";
        return;
    }

    CFStringRef path = device.udi().toCFString();
    io_registry_entry_t entry = IORegistryEntryCopyFromPath(kIOMasterPortDefault, path);
    CFRelease(path);

    if (entry == MACH_PORT_NULL) {
        qWarning() << Q_FUNC_INFO << "Tried to create Device from invalid UDI" << device.udi();
        return;
    }

    d->init(device.udi(), entry);
}

IOKitDevice::~IOKitDevice()
{
    delete d;
}

bool IOKitDevice::conformsToIOKitClass(const QString &className) const
{
    bool conforms = false;
    if (!className.isEmpty()) {
        CFStringRef path = udi().toCFString();
        io_registry_entry_t entry = IORegistryEntryCopyFromPath(kIOMasterPortDefault, path);
        CFRelease(path);
        if (entry != MACH_PORT_NULL) {
            conforms = IOObjectConformsTo(entry, className.toLocal8Bit().constData());
            IOObjectRelease(entry);
        }
    }
    return conforms;
}

QString IOKitDevice::udi() const
{
    return d->udi;
}

QString IOKitDevice::parentUdi() const
{
    return d->parentUdi;
}

QString IOKitDevice::vendor() const
{
    if (parentUdi().isEmpty()) {
        return QStringLiteral("Apple");
    }
    switch (d->mainType) {
    case Solid::DeviceInterface::Processor:
        return Processor::vendor();
        break;
    case Solid::DeviceInterface::Battery:
        return property(QStringLiteral("Manufacturer")).toString();
        break;
    case Solid::DeviceInterface::StorageDrive:
    case Solid::DeviceInterface::OpticalDrive:
    case Solid::DeviceInterface::OpticalDisc:
        return IOKitStorage(this).vendor();
        break;
    case Solid::DeviceInterface::StorageVolume:
        return IOKitVolume(this).vendor();
        break;
    default:
        return QString();
        break;
    }
    return QString();
}

QString IOKitDevice::product() const
{
    if (parentUdi().isEmpty()) {
        return computerModel();
    }
    switch (d->mainType) {
    case Solid::DeviceInterface::Processor:
        return Processor::product();
        break;
    case Solid::DeviceInterface::Battery:
        return property(QStringLiteral("DeviceName")).toString();
        break;
    case Solid::DeviceInterface::StorageDrive:
    case Solid::DeviceInterface::OpticalDrive:
    case Solid::DeviceInterface::OpticalDisc:
        return IOKitStorage(this).product();
        break;
    case Solid::DeviceInterface::StorageVolume:
        return IOKitVolume(this).product();
        break;
    }
    return QString(); // TODO
}

QString IOKitDevice::description() const
{
    switch (d->mainType) {
    case Solid::DeviceInterface::Processor:
        return QStringLiteral("Processor");
        break;
    case Solid::DeviceInterface::Battery:
        return QStringLiteral("Apple Smart Battery");
        break;
    case Solid::DeviceInterface::StorageDrive:
    case Solid::DeviceInterface::OpticalDrive:
    case Solid::DeviceInterface::OpticalDisc:
        return IOKitStorage(this).description();
        break;
    case Solid::DeviceInterface::StorageVolume: {
        const QString volLabel = IOKitVolume(this).description();
        const QString mountPoint = IOKitStorageAccess(this).filePath();
        if (volLabel.isEmpty()) {
            return QUrl::fromLocalFile(mountPoint).fileName();
        } else if (mountPoint.startsWith(QStringLiteral("/Volumes/"))) {
            // Mac users will expect to see the name under which the volume is mounted here.
            return QString(QStringLiteral("%1 (%2)")).arg(QUrl::fromLocalFile(mountPoint).fileName()).arg(volLabel);
        }
        return volLabel;
        break;
    }
    }
    return product(); // TODO
}

QString IOKitDevice::icon() const
{
    // adapted from HalDevice::icon()
    if (parentUdi().isEmpty()) {
        if (computerModel().contains(QStringLiteral("MacBook"))) {
            return QStringLiteral("computer-laptop");
        } else {
            return QStringLiteral("computer");
        }

    } else if (d->type.contains(Solid::DeviceInterface::StorageDrive)) {
        IOKitStorage drive(this);
        Solid::StorageDrive::DriveType driveType = drive.driveType();

        switch (driveType) {
        case Solid::StorageDrive::Floppy:
            // why not :)
            return QStringLiteral("media-floppy");
            break;
        case Solid::StorageDrive::CdromDrive: {
            const IOKitOpticalDisc disc(this);
            if (disc.availableContent() == Solid::OpticalDisc::Audio) {
                return QStringLiteral("media-optical-audio");
            } else
                switch (disc.discType()) {
                case Solid::OpticalDisc::CdRom:
                    return QStringLiteral("media-optical-data");
                    break;
                case Solid::OpticalDisc::CdRecordable:
                case Solid::OpticalDisc::CdRewritable:
                    return QStringLiteral("media-optical-recordable");
                    break;
                case Solid::OpticalDisc::BluRayRom:
                case Solid::OpticalDisc::BluRayRecordable:
                case Solid::OpticalDisc::BluRayRewritable:
                    return QStringLiteral("media-optical-blu-ray");
                    break;
                }
            break;
        }
        case Solid::StorageDrive::SdMmc:
            return QStringLiteral("media-flash-sd-mmc");
            break;
        case Solid::StorageDrive::CompactFlash:
            return QStringLiteral("media-flash-cf");
            break;
        }
        if (drive.bus() == Solid::StorageDrive::Usb) {
            return QStringLiteral("drive-removable-media-usb");
        }
        if (drive.isRemovable()) {
            return QStringLiteral("drive-removable-media");
        }
        return QStringLiteral("drive-harddisk");

    } else if (d->mainType == Solid::DeviceInterface::StorageVolume) {
    } else if (d->mainType == Solid::DeviceInterface::Battery) {
        return QStringLiteral("battery");
    } else if (d->mainType == Solid::DeviceInterface::Processor) {
        return QStringLiteral("cpu"); // FIXME: Doesn't follow icon spec
    } else {
        QString iconName = d->getParentDevice()->icon();

        if (!iconName.isEmpty()) {
            return iconName;
        }

        return QStringLiteral("drive-harddisk");
    }
    return QString();
}

QStringList IOKitDevice::emblems() const
{
    return QStringList(); // TODO
}

QVariant IOKitDevice::property(const QString &key) const
{
    if (!d->properties.contains(key)) {
        return QObject::property(key.toUtf8());
    }
    return d->properties.value(key);
}

QMap<QString, QVariant> IOKitDevice::allProperties() const
{
    return d->properties;
}

bool IOKitDevice::iOKitPropertyExists(const QString &key) const
{
    return d->properties.contains(key);
}

bool IOKitDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    switch (type) {
    case Solid::DeviceInterface::GenericInterface:
        return true;
        break;
    case Solid::DeviceInterface::StorageAccess:
        if (d->type.contains(Solid::DeviceInterface::StorageDrive) //
            || d->type.contains(Solid::DeviceInterface::StorageVolume)) {
            return true;
        }
        break;
    default:
        return d->type.contains(type);
        break;
    }
    return false;
}

QObject *IOKitDevice::createDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    QObject *iface = nullptr;

    switch (type) {
    case Solid::DeviceInterface::GenericInterface:
        iface = new GenericInterface(this);
        break;
    case Solid::DeviceInterface::Processor:
        if (d->type.contains(Solid::DeviceInterface::Processor)) {
            iface = new Processor(this);
        }
        break;
    case Solid::DeviceInterface::Battery:
        if (d->type.contains(Solid::DeviceInterface::Battery)) {
            iface = new Battery(this);
        }
        break;
    case Solid::DeviceInterface::OpticalDrive:
        if (d->type.contains(Solid::DeviceInterface::OpticalDrive)) {
            iface = new IOKitOpticalDrive(this);
        }
        break;
    case Solid::DeviceInterface::OpticalDisc:
        if (d->type.contains(Solid::DeviceInterface::OpticalDisc)) {
            iface = new IOKitOpticalDisc(this);
        }
        break;
    case Solid::DeviceInterface::StorageDrive:
        if (d->type.contains(Solid::DeviceInterface::StorageDrive)) {
            iface = new IOKitStorage(this);
        }
        break;
    case Solid::DeviceInterface::Block:
        if (d->type.contains(Solid::DeviceInterface::OpticalDisc)) {
            iface = new IOKitOpticalDisc(this);
        } else if (d->type.contains(Solid::DeviceInterface::OpticalDrive)) {
            iface = new IOKitOpticalDrive(this);
        } else if (d->type.contains(Solid::DeviceInterface::StorageVolume)) {
            iface = new IOKitVolume(this);
        } else if (d->type.contains(Solid::DeviceInterface::StorageDrive)) {
            iface = new IOKitStorage(this);
        }
        break;
    case Solid::DeviceInterface::StorageVolume:
        if (d->type.contains(Solid::DeviceInterface::StorageVolume)) {
            iface = new IOKitVolume(this);
        }
        break;
    case Solid::DeviceInterface::StorageAccess:
        if (d->type.contains(Solid::DeviceInterface::StorageDrive) //
            || d->type.contains(Solid::DeviceInterface::StorageVolume)) {
            iface = new IOKitStorageAccess(this);
        }
        break;
        // the rest is TODO
    }

    return iface;
}

}
}
} // namespaces

#include "moc_iokitdevice.cpp"
