/*
    SPDX-FileCopyrightText: 2010 Michael Zanetti <mzanetti@kde.org>
    SPDX-FileCopyrightText: 2010 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "upowermanager.h"
#include "upower.h"
#include "upowerdevice.h"

#include <QDBusConnectionInterface>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QDebug>

#include "../shared/rootdevice.h"

using namespace Solid::Backends::UPower;
using namespace Solid::Backends::Shared;

UPowerManager::UPowerManager(QObject *parent)
    : Solid::Ifaces::DeviceManager(parent)
    , m_supportedInterfaces({Solid::DeviceInterface::GenericInterface, Solid::DeviceInterface::Battery})
    , m_manager(QDBusConnection::systemBus())
    , m_knownDevices(udiPrefix())
{
    qDBusRegisterMetaType<QList<QDBusObjectPath>>();
    qDBusRegisterMetaType<QVariantMap>();

    bool serviceFound = m_manager.isValid();
    if (!serviceFound) {
        // find out whether it will be activated automatically
        QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.DBus", //
                                                              "/org/freedesktop/DBus",
                                                              "org.freedesktop.DBus",
                                                              "ListActivatableNames");

        QDBusReply<QStringList> reply = QDBusConnection::systemBus().call(message);
        if (reply.isValid() && reply.value().contains(UP_DBUS_SERVICE)) {
            QDBusConnection::systemBus().interface()->startService(UP_DBUS_SERVICE);
            serviceFound = true;
        }
    }

    if (serviceFound) {
        connect(&m_manager, &UPower::DBusInterface::DeviceAdded, this, &UPowerManager::onDeviceAdded);
        connect(&m_manager, &UPower::DBusInterface::DeviceRemoved, this, &UPowerManager::onDeviceRemoved);
    }
}

UPowerManager::~UPowerManager()
{
}

QObject *UPowerManager::createDevice(const QString &udi)
{
    if (udi == udiPrefix()) {
        RootDevice *root = new RootDevice(udiPrefix());

        root->setProduct(tr("Power Management"));
        root->setDescription(tr("Batteries and other sources of power"));
        root->setIcon("preferences-system-power-management");

        return root;

    } else if (m_knownDevices.contains(udi) || allDevices().contains(udi)) {
        return new UPowerDevice(udi);

    } else {
        return nullptr;
    }
}

QStringList UPowerManager::devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type)
{
    const QStringList allDev = allDevices();
    QStringList result;

    if (!parentUdi.isEmpty()) {
        for (const QString &udi : allDev) {
            if (udi == udiPrefix()) {
                continue;
            }

            UPowerDevice device(udi);
            if (device.queryDeviceInterface(type) && device.parentUdi() == parentUdi) {
                result << udi;
            }
        }

        return result;
    } else if (type != Solid::DeviceInterface::Unknown) {
        for (const QString &udi : allDev) {
            if (udi == udiPrefix()) {
                continue;
            }

            UPowerDevice device(udi);
            if (device.queryDeviceInterface(type)) {
                result << udi;
            }
        }

        return result;
    } else {
        return allDev;
    }
}

QStringList UPowerManager::allDevices()
{
    static bool initialized = false;
    if (initialized)
        return m_knownDevices;

    auto reply = m_manager.EnumerateDevices();
    reply.waitForFinished();

    if (!reply.isValid()) {
        qWarning() << Q_FUNC_INFO << " error: " << reply.error().name();
        return QStringList();
    }
    const auto pathList = reply.value();

    QStringList retList;
    retList.reserve(pathList.size() + m_knownDevices.size());

    for (const QDBusObjectPath &path : pathList) {
        retList << path.path();
    }
    retList += m_knownDevices;

    std::sort(retList.begin(), retList.end());
    auto end = std::unique(retList.begin(), retList.end());
    retList.erase(end, retList.end());

    m_knownDevices = retList;
    initialized = true;

    return m_knownDevices;
}

QSet<Solid::DeviceInterface::Type> UPowerManager::supportedInterfaces() const
{
    return m_supportedInterfaces;
}

QString UPowerManager::udiPrefix() const
{
    return UP_UDI_PREFIX;
}

void UPowerManager::onDeviceAdded(const QDBusObjectPath &path)
{
    auto pathString = path.path();
    if (m_knownDevices.indexOf(pathString) < 0)
        m_knownDevices.append(pathString);

    Q_EMIT deviceAdded(pathString);
}

void UPowerManager::onDeviceRemoved(const QDBusObjectPath &path)
{
    auto pathString = path.path();
    auto index = m_knownDevices.indexOf(pathString);
    if (index >= 0) {
        m_knownDevices.removeAt(index);
        Q_EMIT deviceRemoved(pathString);
    }
}

#include "moc_upowermanager.cpp"
