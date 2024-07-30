/*
    SPDX-FileCopyrightText: 2010 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udevmanager.h"

#include "../shared/rootdevice.h"
#include "udev.h"
#include "udevdevice.h"

#include <QDebug>
#include <QFile>
#include <QSet>

using namespace Solid::Backends::UDev;
using namespace Solid::Backends::Shared;

class UDevManager::Private
{
public:
    Private();
    ~Private();

    bool isOfInterest(const QString &udi, const UdevQt::Device &device);
    bool checkOfInterest(const UdevQt::Device &device);

    UdevQt::Client *m_client;
    QStringList m_devicesOfInterest;
    QSet<Solid::DeviceInterface::Type> m_supportedInterfaces;
};

UDevManager::Private::Private()
{
    QStringList subsystems;
    subsystems << "processor";
    subsystems << "cpu";
    subsystems << "sound";
    subsystems << "tty";
    subsystems << "dvb";
    subsystems << "net";
    subsystems << "usb";
    subsystems << "input";
    m_client = new UdevQt::Client(subsystems);
}

UDevManager::Private::~Private()
{
    delete m_client;
}

bool UDevManager::Private::isOfInterest(const QString &udi, const UdevQt::Device &device)
{
    if (m_devicesOfInterest.contains(udi)) {
        return true;
    }

    bool isOfInterest = checkOfInterest(device);
    if (isOfInterest) {
        m_devicesOfInterest.append(udi);
    }

    return isOfInterest;
}

bool UDevManager::Private::checkOfInterest(const UdevQt::Device &device)
{
    const QString subsystem = device.subsystem();
#ifdef UDEV_DETAILED_OUTPUT
    qDebug() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
    qDebug() << "Path:" << device.sysfsPath();
    qDebug() << "Properties:" << device.deviceProperties();

    const QStringList properties = device.deviceProperties();
    for (const QString &key : properties) {
        qDebug() << "\t" << key << ":" << device.deviceProperty(key).toString();
    }
    qDebug() << "Driver:" << device.driver();
    qDebug() << "Subsystem:" << subsystem;
    qDebug() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
#endif

    if (subsystem == QLatin1String("cpu")) {
        // Linux ACPI reports processor slots, rather than processors.
        // Empty slots will not have a system device associated with them.
        return QFile::exists(device.sysfsPath() + "/sysdev") //
            || QFile::exists(device.sysfsPath() + "/cpufreq") //
            || QFile::exists(device.sysfsPath() + "/topology/core_id");
    }
    if (subsystem == QLatin1String("sound") && device.deviceProperty("SOUND_FORM_FACTOR").toString() != "internal") {
        return true;
    }

    if (subsystem == QLatin1String("tty")) {
        QString path = device.deviceProperty("DEVPATH").toString();

        int lastSlash = path.length() - path.lastIndexOf(QLatin1String("/")) - 1;
        QByteArray lastElement = path.right(lastSlash).toLatin1();

        if (lastElement.startsWith("tty") && !path.startsWith("/devices/virtual")) {
            return true;
        }
    }

    if (subsystem == QLatin1String("input")) {
        /* clang-format off */
        if (device.deviceProperty("ID_INPUT_MOUSE").toInt() == 1
            || device.deviceProperty("ID_INPUT_TOUCHPAD").toInt() == 1
            || device.deviceProperty("ID_INPUT_TABLET").toInt() == 1
            || device.deviceProperty("ID_INPUT_JOYSTICK").toInt() == 1
            || device.deviceProperty("ID_INPUT_TOUCHSCREEN").toInt() == 1) { /* clang-format on */
            return true;
        }
    }

    /* clang-format off */
    return subsystem == QLatin1String("dvb")
        || subsystem == QLatin1String("net")
        || (!device.deviceProperty("ID_MEDIA_PLAYER").toString().isEmpty()
            && device.parent().deviceProperty("ID_MEDIA_PLAYER").toString().isEmpty()) // media-player-info recognized devices
        || (device.deviceProperty("ID_GPHOTO2").toInt() == 1
            && device.parent().deviceProperty("ID_GPHOTO2").toInt() != 1); // GPhoto2 cameras
    /* clang-format on */
}

UDevManager::UDevManager(QObject *parent)
    : Solid::Ifaces::DeviceManager(parent)
    , d(new Private)
{
    connect(d->m_client, SIGNAL(deviceAdded(UdevQt::Device)), this, SLOT(slotDeviceAdded(UdevQt::Device)));
    connect(d->m_client, SIGNAL(deviceRemoved(UdevQt::Device)), this, SLOT(slotDeviceRemoved(UdevQt::Device)));

    // clang-format off
    d->m_supportedInterfaces << Solid::DeviceInterface::GenericInterface
                             << Solid::DeviceInterface::Processor
                             << Solid::DeviceInterface::Camera
                             << Solid::DeviceInterface::PortableMediaPlayer
                             << Solid::DeviceInterface::Block;
    // clang-format on
}

UDevManager::~UDevManager()
{
    delete d;
}

QString UDevManager::udiPrefix() const
{
    return QString::fromLatin1(UDEV_UDI_PREFIX);
}

QSet<Solid::DeviceInterface::Type> UDevManager::supportedInterfaces() const
{
    return d->m_supportedInterfaces;
}

QStringList UDevManager::allDevices()
{
    QStringList res;
    const UdevQt::DeviceList deviceList = d->m_client->allDevices();
    for (const UdevQt::Device &device : deviceList) {
        if (d->isOfInterest(udiPrefix() + device.sysfsPath(), device)) {
            res << udiPrefix() + device.sysfsPath();
        }
    }
    return res;
}

QStringList UDevManager::devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type)
{
    QStringList result;

    if (!parentUdi.isEmpty()) {
        const UdevQt::DeviceList deviceList = d->m_client->allDevices();
        for (const UdevQt::Device &dev : deviceList) {
            UDevDevice device(dev);
            if (device.queryDeviceInterface(type) && d->isOfInterest(udiPrefix() + dev.sysfsPath(), dev) && device.parentUdi() == parentUdi) {
                result << udiPrefix() + dev.sysfsPath();
            }
        }
        return result;
    }

    if (type == DeviceInterface::Unknown) {
        return allDevices();
    }

    UdevQt::DeviceList deviceList;

    // Already limit the number of devices we query and have to create wrapper items for here
    if (type == Solid::DeviceInterface::Processor) {
        deviceList = (d->m_client->devicesBySubsystem(QStringLiteral("processor")) //
                      + d->m_client->devicesBySubsystem(QStringLiteral("cpu")));
    } else if (type == Solid::DeviceInterface::Camera) {
        deviceList = d->m_client->devicesBySubsystemsAndProperties({QStringLiteral("usb")}, //
                                                                   {{QStringLiteral("ID_GPHOTO2"), QStringLiteral("*")}}); // match any
    } else if (type == Solid::DeviceInterface::PortableMediaPlayer) {
        deviceList = d->m_client->devicesBySubsystemsAndProperties({QStringLiteral("usb")}, //
                                                                   {{QStringLiteral("ID_MEDIA_PLAYER"), QStringLiteral("*")}}); // match any
    } else {
        deviceList = d->m_client->allDevices();
    }

    for (const UdevQt::Device &dev : std::as_const(deviceList)) {
        UDevDevice device(dev);
        if (device.queryDeviceInterface(type) //
            && d->isOfInterest(udiPrefix() + dev.sysfsPath(), dev)) {
            result << udiPrefix() + dev.sysfsPath();
        }
    }

    return result;
}

QObject *UDevManager::createDevice(const QString &udi_)
{
    if (udi_ == udiPrefix()) {
        RootDevice *const device = new RootDevice(UDEV_UDI_PREFIX);
        device->setProduct(tr("Devices"));
        device->setDescription(tr("Devices declared in your system"));
        device->setIcon("computer");

        return device;
    }

    const QString udi = udi_.right(udi_.size() - udiPrefix().size());
    UdevQt::Device device = d->m_client->deviceBySysfsPath(udi);

    if (d->isOfInterest(udi_, device) || QFile::exists(udi)) {
        return new UDevDevice(device);
    }

    return nullptr;
}

void UDevManager::slotDeviceAdded(const UdevQt::Device &device)
{
    if (d->isOfInterest(udiPrefix() + device.sysfsPath(), device)) {
        Q_EMIT deviceAdded(udiPrefix() + device.sysfsPath());
    }
}

void UDevManager::slotDeviceRemoved(const UdevQt::Device &device)
{
    if (d->isOfInterest(udiPrefix() + device.sysfsPath(), device)) {
        Q_EMIT deviceRemoved(udiPrefix() + device.sysfsPath());
        d->m_devicesOfInterest.removeAll(udiPrefix() + device.sysfsPath());
    }
}

#include "moc_udevmanager.cpp"
