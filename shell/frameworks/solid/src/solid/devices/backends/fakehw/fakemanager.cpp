/*
    SPDX-FileCopyrightText: 2006 Michaël Larouche <michael.larouche@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "fakemanager.h"

#include "fakedevice.h"

// Qt includes
#include <QDebug>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QFile>
#include <QString>
#ifdef HAVE_DBUS
#include <QDBusConnection>
#endif

using namespace Solid::Backends::Fake;

class FakeManager::Private
{
public:
    QMap<QString, FakeDevice *> loadedDevices;
    QMap<QString, QMap<QString, QVariant>> hiddenDevices;
    QString xmlFile;
    QSet<Solid::DeviceInterface::Type> supportedInterfaces;
};

FakeManager::FakeManager(QObject *parent, const QString &xmlFile)
    : Solid::Ifaces::DeviceManager(parent)
    , d(new Private)
{
    QString machineXmlFile = xmlFile;
    d->xmlFile = machineXmlFile;

#ifdef HAVE_DBUS
    QDBusConnection::sessionBus().registerObject("/org/kde/solid/fakehw", this, QDBusConnection::ExportNonScriptableSlots);
#endif

    parseMachineFile();

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
                           << Solid::DeviceInterface::Battery
                           << Solid::DeviceInterface::NetworkShare;
    // clang-format on
}

FakeManager::~FakeManager()
{
#ifdef HAVE_DBUS
    QDBusConnection::sessionBus().unregisterObject("/org/kde/solid/fakehw", QDBusConnection::UnregisterTree);
#endif
    qDeleteAll(d->loadedDevices);
    delete d;
}

QString FakeManager::udiPrefix() const
{
    return "/org/kde/solid/fakehw";
}

QSet<Solid::DeviceInterface::Type> FakeManager::supportedInterfaces() const
{
    return d->supportedInterfaces;
}

QStringList FakeManager::allDevices()
{
    QStringList deviceUdiList;

    for (const FakeDevice *device : std::as_const(d->loadedDevices)) {
        deviceUdiList.append(device->udi());
    }

    return deviceUdiList;
}

QStringList FakeManager::devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type)
{
    if (!parentUdi.isEmpty()) {
        QStringList found = findDeviceStringMatch(QLatin1String("parent"), parentUdi);

        if (type == Solid::DeviceInterface::Unknown) {
            return found;
        }

        QStringList result;

        QStringList::ConstIterator it = found.constBegin();
        QStringList::ConstIterator end = found.constEnd();

        for (; it != end; ++it) {
            FakeDevice *device = d->loadedDevices[*it];

            if (device->queryDeviceInterface(type)) {
                result << *it;
            }
        }

        return result;
    } else if (type != Solid::DeviceInterface::Unknown) {
        return findDeviceByDeviceInterface(type);
    } else {
        return allDevices();
    }
}

QObject *FakeManager::createDevice(const QString &udi)
{
    if (d->loadedDevices.contains(udi)) {
        return new FakeDevice(*d->loadedDevices[udi]);
    }

    return nullptr;
}

FakeDevice *FakeManager::findDevice(const QString &udi)
{
    return d->loadedDevices.value(udi);
}

QStringList FakeManager::findDeviceStringMatch(const QString &key, const QString &value)
{
    QStringList result;
    for (const FakeDevice *device : std::as_const(d->loadedDevices)) {
        if (device->property(key).toString() == value) {
            result.append(device->udi());
        }
    }

    return result;
}

QStringList FakeManager::findDeviceByDeviceInterface(Solid::DeviceInterface::Type type)
{
    QStringList result;
    for (const FakeDevice *device : std::as_const(d->loadedDevices)) {
        if (device->queryDeviceInterface(type)) {
            result.append(device->udi());
        }
    }

    return result;
}

void FakeManager::plug(const QString &udi)
{
    if (d->hiddenDevices.contains(udi)) {
        QMap<QString, QVariant> properties = d->hiddenDevices.take(udi);
        d->loadedDevices[udi] = new FakeDevice(udi, properties);
        Q_EMIT deviceAdded(udi);
    }
}

void FakeManager::unplug(const QString &udi)
{
    if (d->loadedDevices.contains(udi)) {
        FakeDevice *dev = d->loadedDevices.take(udi);
        d->hiddenDevices[udi] = dev->allProperties();
        Q_EMIT deviceRemoved(udi);
        delete dev;
    }
}

void FakeManager::parseMachineFile()
{
    QFile machineFile(d->xmlFile);
    if (!machineFile.open(QIODevice::ReadOnly)) {
        qWarning() << Q_FUNC_INFO << "Error while opening " << d->xmlFile;
        return;
    }

    QDomDocument fakeDocument;
    if (!fakeDocument.setContent(&machineFile)) {
        qWarning() << Q_FUNC_INFO << "Error while creating the QDomDocument.";
        machineFile.close();
        return;
    }
    machineFile.close();

    qDebug() << Q_FUNC_INFO << "Parsing fake computer XML: " << d->xmlFile;
    QDomElement mainElement = fakeDocument.documentElement();
    QDomNode node = mainElement.firstChild();
    while (!node.isNull()) {
        QDomElement tempElement = node.toElement();
        if (!tempElement.isNull() && tempElement.tagName() == QLatin1String("device")) {
            FakeDevice *tempDevice = parseDeviceElement(tempElement);
            if (tempDevice) {
                Q_ASSERT(!d->loadedDevices.contains(tempDevice->udi()));
                d->loadedDevices.insert(tempDevice->udi(), tempDevice);
                Q_EMIT deviceAdded(tempDevice->udi());
            }
        }

        node = node.nextSibling();
    }
}

FakeDevice *FakeManager::parseDeviceElement(const QDomElement &deviceElement)
{
    FakeDevice *device = nullptr;
    QMap<QString, QVariant> propertyMap;
    QString udi = deviceElement.attribute("udi");

    QDomNode propertyNode = deviceElement.firstChild();
    while (!propertyNode.isNull()) {
        QDomElement propertyElement = propertyNode.toElement();
        if (!propertyElement.isNull() && propertyElement.tagName() == QLatin1String("property")) {
            QString propertyKey;
            QVariant propertyValue;

            propertyKey = propertyElement.attribute("key");
            propertyValue = QVariant(propertyElement.text());

            propertyMap.insert(propertyKey, propertyValue);
        }

        propertyNode = propertyNode.nextSibling();
    }

    if (!propertyMap.isEmpty()) {
        device = new FakeDevice(udi, propertyMap);
    }

    return device;
}

#include "moc_fakemanager.cpp"
