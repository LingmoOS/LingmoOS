/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "windevice.h"
#include <solid/deviceinterface.h>

#include "winbattery.h"
#include "winblock.h"
#include "wingenericinterface.h"
#include "winopticaldisc.h"
#include "winopticaldrive.h"
#include "winprocessor.h"
#include "winstorageaccess.h"
#include "winstoragedrive.h"
#include "winstoragevolume.h"

#include <batclass.h>

#if defined(__MINGW32__) && !defined(IOCTL_STORAGE_QUERY_PROPERTY)
#include <winioctl_backport.h>
#endif

using namespace Solid::Backends::Win;

WinDevice::WinDevice(const QString &udi)
    : Device()
    , m_udi(udi)
    , m_type(Solid::DeviceInterface::Unknown)
{
    /*
     /org/kde/solid/win/volume/disk #%1, partition #%2
     /org/kde/solid/win/storage.cdrom/disk #%0
    */
    QStringList data = udi.split("/");
    QString parentName = data[6].split(",")[0].trimmed();
    QString type = data[5];

    if (type == "storage") {
        m_type = Solid::DeviceInterface::StorageDrive;
    } else if (type == "volume") {
        m_type = Solid::DeviceInterface::StorageVolume;
    } else if (type == "storage.cdrom") {
        m_type = Solid::DeviceInterface::OpticalDrive;
    } else if (type == "volume.cdrom") {
        m_type = Solid::DeviceInterface::OpticalDisc;
    } else if (type == "cpu") {
        m_type = Solid::DeviceInterface::Processor;
    } else if (type == "power.battery") {
        m_type = Solid::DeviceInterface::Battery;
    } else if (type == "volume.virtual") {
        m_type = Solid::DeviceInterface::StorageAccess;
    }

    switch (m_type) {
    case Solid::DeviceInterface::StorageVolume: {
        m_parentUdi = QLatin1String("/org/kde/solid/win/storage/") + parentName;
        break;
    }
    case Solid::DeviceInterface::OpticalDisc: {
        m_parentUdi = QLatin1String("/org/kde/solid/win/storage.cdrom/") + parentName;
        break;
    }
    case Solid::DeviceInterface::StorageAccess: {
        m_parentUdi = WinBlock::udiFromDriveLetter(WinBlock::resolveVirtualDrive(udi).mid(0, 2));
        if (m_parentUdi.isEmpty()) {
            m_parentUdi = QLatin1String("/org/kde/solid/win/") + type;
        }
        break;
    }
    default:
        m_parentUdi = QLatin1String("/org/kde/solid/win/") + type;
    }

    switch (m_type) {
    case Solid::DeviceInterface::Processor:
        initCpuDevice();
        break;
    case Solid::DeviceInterface::Battery:
        initBatteryDevice();
        break;
    default:
        if (queryDeviceInterface(Solid::DeviceInterface::StorageAccess) || queryDeviceInterface(Solid::DeviceInterface::StorageDrive)) {
            initStorageDevice();
        } else {
            qWarning() << "Unknown device" << udi;
        }
    }
}

void WinDevice::initStorageDevice()
{
    QString dev;
    switch (m_type) {
    case Solid::DeviceInterface::StorageAccess:
        dev = WinBlock::driveLetterFromUdi(udi());
        m_product = QString("Virtual drive %1").arg(dev);
        m_description = QString("%1 (%2)").arg(dev, WinBlock::resolveVirtualDrive(udi()));
        return;
    case Solid::DeviceInterface::OpticalDrive:
        dev = WinBlock::driveLetterFromUdi(udi());
        break;
    case Solid::DeviceInterface::StorageDrive:
        dev = QString("PhysicalDrive%1").arg(WinBlock(this).deviceMajor());
        break;
    default:
        dev = WinBlock::driveLetterFromUdi(udi());
        const QString label = WinStorageVolume(this).label();
        if (!label.isEmpty()) {
            m_description = QStringLiteral("%1 (%2)").arg(dev, label);
        } else {
            m_description = dev;
        }
    }

    STORAGE_PROPERTY_QUERY query;
    ZeroMemory(&query, sizeof(STORAGE_PROPERTY_QUERY));
    query.PropertyId = StorageDeviceProperty;
    query.QueryType = PropertyStandardQuery;

    char buff[1024];
    WinDeviceManager::getDeviceInfo<char, STORAGE_PROPERTY_QUERY>(dev, IOCTL_STORAGE_QUERY_PROPERTY, buff, 1024, &query);
    STORAGE_DEVICE_DESCRIPTOR *info = ((STORAGE_DEVICE_DESCRIPTOR *)buff);
    if (info->VendorIdOffset != 0) {
        m_vendor = QString((char *)buff + info->VendorIdOffset).trimmed();
        if (info->ProductIdOffset != 0) {
            m_product = QString((char *)buff + info->ProductIdOffset).trimmed();
        }
    } else if (info->ProductIdOffset != 0) { // fallback doesn't work for all devices
        QStringList tmp = QString((char *)buff + info->ProductIdOffset).trimmed().split(" ");
        m_vendor = tmp.takeFirst();
        m_product = tmp.join(" ");
    }
}

void WinDevice::initBatteryDevice()
{
    WinBattery::Battery battery = WinBattery::batteryInfoFromUdi(m_udi);
    BATTERY_QUERY_INFORMATION query;
    ZeroMemory(&query, sizeof(query));
    query.BatteryTag = battery.second;

    DWORD size = 1024;
    wchar_t buff[1024];

    query.InformationLevel = BatteryDeviceName;
    WinDeviceManager::getDeviceInfo<wchar_t, BATTERY_QUERY_INFORMATION>(battery.first, IOCTL_BATTERY_QUERY_INFORMATION, buff, size, &query);
    m_product = QString::fromWCharArray(buff);

    query.InformationLevel = BatteryManufactureName;
    WinDeviceManager::getDeviceInfo<wchar_t, BATTERY_QUERY_INFORMATION>(battery.first, IOCTL_BATTERY_QUERY_INFORMATION, buff, size, &query);
    m_vendor = QString::fromWCharArray(buff);

    switch (WinBattery(this).technology()) {
    case Solid::Battery::LithiumIon:
        m_description = tr("Lithium Ion", "battery technology");
        break;
    case Solid::Battery::LeadAcid:
        m_description = tr("Lead Acid", "battery technology");
        break;
    case Solid::Battery::NickelCadmium:
        m_description = tr("Nickel Cadmium", "battery technology");
        break;
    case Solid::Battery::NickelMetalHydride:
        m_description = tr("Nickel Metal Hydride", "battery technology");
        break;
    default:
        m_description = tr("Unknown", "battery technology");
    }
}

void WinDevice::initCpuDevice()
{
    WinProcessor cpu(this);
    WinProcessor::ProcessorInfo info = WinProcessor::updateCache()[cpu.number()];
    m_vendor = info.vendor;
    m_product = info.produuct;
    m_description = info.name;
}

QString WinDevice::udi() const
{
    return m_udi;
}

QString WinDevice::parentUdi() const
{
    return m_parentUdi;
}

QString WinDevice::vendor() const
{
    return m_vendor;
}

QString WinDevice::product() const
{
    return m_product;
}

QString WinDevice::description() const
{
    return m_description.isEmpty() ? m_product : m_description;
}

QString WinDevice::icon() const
{
    if (parentUdi().isEmpty()) {
        return QLatin1String("computer");
    }

    QString icon;
    switch (type()) {
    case Solid::DeviceInterface::OpticalDrive:
        icon = QLatin1String("drive-optical");
        break;
    case Solid::DeviceInterface::OpticalDisc: {
        WinOpticalDisc disk(const_cast<WinDevice *>(this));
        if (disk.availableContent() & Solid::OpticalDisc::Audio) {
            icon = QLatin1String("media-optical-audio");
        } else {
            icon = QLatin1String("drive-optical");
        }
        break;
    }
    case Solid::DeviceInterface::StorageDrive:
    case Solid::DeviceInterface::StorageVolume: {
        WinStorageDrive storage(const_cast<WinDevice *>(this));
        if (storage.bus() == Solid::StorageDrive::Usb) {
            icon = QLatin1String("drive-removable-media-usb-pendrive");
        } else {
            icon = QLatin1String("drive-harddisk");
        }
        break;
    }
    case Solid::DeviceInterface::Processor:
        icon = QLatin1String("cpu");
        break;
    case Solid::DeviceInterface::Battery:
        icon = QLatin1String("battery");
        break;
    case Solid::DeviceInterface::StorageAccess:
        icon = QLatin1String("drive-harddisk");
        break;
    default:
        break;
    }
    return icon;
}

QStringList WinDevice::emblems() const
{
    QStringList icons;
    switch (type()) {
    case Solid::DeviceInterface::StorageAccess:
        icons << QLatin1String("emblem-symbolic-link");
        break;
    default:
        break;
    }
    return icons;
}

bool WinDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &queryType) const
{
    if (queryType == Solid::DeviceInterface::GenericInterface) {
        return true;
    }

    QList<Solid::DeviceInterface::Type> interfaceList;
    interfaceList << type();

    switch (type()) {
    case Solid::DeviceInterface::GenericInterface:
        break;
    case Solid::DeviceInterface::Block:
        break;
    case Solid::DeviceInterface::StorageAccess:
        break;
    case Solid::DeviceInterface::StorageDrive:
        break;
    case Solid::DeviceInterface::OpticalDrive:
        interfaceList << Solid::DeviceInterface::Block << Solid::DeviceInterface::StorageDrive;
        break;
    case Solid::DeviceInterface::StorageVolume:
        interfaceList << Solid::DeviceInterface::Block << Solid::DeviceInterface::StorageAccess;
        break;
    case Solid::DeviceInterface::OpticalDisc:
        interfaceList << Solid::DeviceInterface::Block << Solid::DeviceInterface::StorageVolume << Solid::DeviceInterface::StorageAccess;
        break;
    case Solid::DeviceInterface::PortableMediaPlayer:
        break;
    case Solid::DeviceInterface::Unknown:
    case Solid::DeviceInterface::Last:
    default:
        break;
    }

    if (interfaceList.size() == 0) {
        qWarning() << "no interface found for type" << type();
    }
    return interfaceList.contains(queryType);
}

QObject *WinDevice::createDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    if (!queryDeviceInterface(type)) {
        return 0;
    }
    WinInterface *iface = 0;

    switch (type) {
    case Solid::DeviceInterface::GenericInterface:
        iface = new WinGenericInterface(this);
        break;
    case Solid::DeviceInterface::Block:
        iface = new WinBlock(this);
        break;
    case Solid::DeviceInterface::Processor:
        iface = new WinProcessor(this);
        break;
    case Solid::DeviceInterface::StorageAccess:
        iface = new WinStorageAccess(this);
        break;
    case Solid::DeviceInterface::StorageDrive:
        iface = new WinStorageDrive(this);
        break;
    case Solid::DeviceInterface::OpticalDrive:
        iface = new WinOpticalDrive(this);
        break;
    case Solid::DeviceInterface::StorageVolume:
        iface = new WinStorageVolume(this);
        break;
    case Solid::DeviceInterface::OpticalDisc:
        iface = new WinOpticalDisc(this);
        break;
    //      case Solid::DeviceInterface::PortableMediaPlayer:
    //          iface = new PortableMediaPlayer(this);
    //          break;
    case Solid::DeviceInterface::Battery:
        iface = new WinBattery(this);
        break;
    case Solid::DeviceInterface::Unknown:
    case Solid::DeviceInterface::Last:
        break;
    default:
        break;
    }

    return iface;
}

Solid::DeviceInterface::Type WinDevice::type() const
{
    return m_type;
}

#include "moc_windevice.cpp"
