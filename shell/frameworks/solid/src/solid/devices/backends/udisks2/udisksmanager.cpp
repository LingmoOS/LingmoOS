/*
    SPDX-FileCopyrightText: 2012 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udisksmanager.h"
#include "udisks_debug.h"
#include "udisksdevicebackend.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QDomDocument>

#include "../shared/rootdevice.h"

using namespace Solid::Backends::UDisks2;
using namespace Solid::Backends::Shared;

Manager::Manager(QObject *parent)
    : Solid::Ifaces::DeviceManager(parent)
    , m_manager(UD2_DBUS_SERVICE, UD2_DBUS_PATH, QDBusConnection::systemBus())
{
    m_supportedInterfaces = {
        Solid::DeviceInterface::GenericInterface,
        Solid::DeviceInterface::Block,
        Solid::DeviceInterface::StorageAccess,
        Solid::DeviceInterface::StorageDrive,
        Solid::DeviceInterface::OpticalDrive,
        Solid::DeviceInterface::OpticalDisc,
        Solid::DeviceInterface::StorageVolume,
    };

    qDBusRegisterMetaType<QList<QDBusObjectPath>>();
    qDBusRegisterMetaType<QVariantMap>();
    qDBusRegisterMetaType<VariantMapMap>();
    qDBusRegisterMetaType<DBUSManagerStruct>();

    bool serviceFound = m_manager.isValid();
    if (!serviceFound) {
        // find out whether it will be activated automatically
        QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.DBus", //
                                                              "/org/freedesktop/DBus",
                                                              "org.freedesktop.DBus",
                                                              "ListActivatableNames");

        QDBusReply<QStringList> reply = QDBusConnection::systemBus().call(message);
        if (reply.isValid() && reply.value().contains(UD2_DBUS_SERVICE)) {
            QDBusConnection::systemBus().interface()->startService(UD2_DBUS_SERVICE);
            serviceFound = true;
        }
    }

    if (serviceFound) {
        connect(&m_manager, SIGNAL(InterfacesAdded(QDBusObjectPath, VariantMapMap)), this, SLOT(slotInterfacesAdded(QDBusObjectPath, VariantMapMap)));
        connect(&m_manager, SIGNAL(InterfacesRemoved(QDBusObjectPath, QStringList)), this, SLOT(slotInterfacesRemoved(QDBusObjectPath, QStringList)));
    }
}

Manager::~Manager()
{
    while (!m_deviceCache.isEmpty()) {
        QString udi = m_deviceCache.takeFirst();
        DeviceBackend::destroyBackend(udi);
    }
}

QObject *Manager::createDevice(const QString &udi)
{
    if (udi == udiPrefix()) {
        RootDevice *root = new RootDevice(udi);

        root->setProduct(tr("Storage"));
        root->setDescription(tr("Storage devices"));
        root->setIcon("server-database"); // Obviously wasn't meant for that, but maps nicely in oxygen icon set :-p

        return root;
    } else if (deviceCache().contains(udi)) {
        return new Device(udi);
    } else {
        return nullptr;
    }
}

QStringList Manager::devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type)
{
    QStringList result;
    const QStringList deviceList = deviceCache();

    if (!parentUdi.isEmpty()) {
        for (const QString &udi : deviceList) {
            Device device(udi);
            if (device.queryDeviceInterface(type) && device.parentUdi() == parentUdi) {
                result << udi;
            }
        }

        return result;
    } else if (type != Solid::DeviceInterface::Unknown) {
        for (const QString &udi : deviceList) {
            Device device(udi);
            if (device.queryDeviceInterface(type)) {
                result << udi;
            }
        }

        return result;
    }

    return deviceCache();
}

QStringList Manager::allDevices()
{
    m_deviceCache.clear();

    introspect(UD2_DBUS_PATH_BLOCKDEVICES, true /*checkOptical*/);
    introspect(UD2_DBUS_PATH_DRIVES);

    return m_deviceCache;
}

void Manager::introspect(const QString &path, bool checkOptical)
{
    QDBusMessage call = QDBusMessage::createMethodCall(UD2_DBUS_SERVICE, path, DBUS_INTERFACE_INTROSPECT, "Introspect");
    QDBusPendingReply<QString> reply = QDBusConnection::systemBus().call(call);

    if (reply.isValid()) {
        QDomDocument dom;
        dom.setContent(reply.value());
        QDomNodeList nodeList = dom.documentElement().elementsByTagName("node");
        for (int i = 0; i < nodeList.count(); i++) {
            QDomElement nodeElem = nodeList.item(i).toElement();
            if (!nodeElem.isNull() && nodeElem.hasAttribute("name")) {
                const QString name = nodeElem.attribute("name");
                const QString udi = path + "/" + name;

                // Optimization, a loop device cannot really have a physical drive associated with it
                if (checkOptical && !name.startsWith(QLatin1String("loop"))) {
                    Device device(udi);
                    if (device.mightBeOpticalDisc()) {
                        QDBusConnection::systemBus().connect(UD2_DBUS_SERVICE, //
                                                             udi,
                                                             DBUS_INTERFACE_PROPS,
                                                             "PropertiesChanged",
                                                             this,
                                                             SLOT(slotMediaChanged(QDBusMessage)));
                        if (!device.isOpticalDisc()) { // skip empty CD disc
                            continue;
                        }
                    }
                }

                m_deviceCache.append(udi);
            }
        }
    } else {
        qCWarning(UDISKS2) << "Failed enumerating UDisks2 objects:" << reply.error().name() << "\n" << reply.error().message();
    }
}

QSet<Solid::DeviceInterface::Type> Manager::supportedInterfaces() const
{
    return m_supportedInterfaces;
}

QString Manager::udiPrefix() const
{
    return UD2_UDI_DISKS_PREFIX;
}

void Manager::slotInterfacesAdded(const QDBusObjectPath &object_path, const VariantMapMap &interfaces_and_properties)
{
    const QString udi = object_path.path();

    /* Ignore jobs */
    if (udi.startsWith(UD2_DBUS_PATH_JOBS)) {
        return;
    }

    qCDebug(UDISKS2) << udi << "has new interfaces:" << interfaces_and_properties.keys();

    // If device gained an org.freedesktop.UDisks2.Block interface, we
    // should check if it is an optical drive, in order to properly
    // register mediaChanged event handler with newly-plugged external
    // drives
    if (interfaces_and_properties.contains("org.freedesktop.UDisks2.Block")) {
        Device device(udi);
        if (device.mightBeOpticalDisc()) {
            QDBusConnection::systemBus().connect(UD2_DBUS_SERVICE, //
                                                 udi,
                                                 DBUS_INTERFACE_PROPS,
                                                 "PropertiesChanged",
                                                 this,
                                                 SLOT(slotMediaChanged(QDBusMessage)));
        }
    }

    updateBackend(udi);

    // new device, we don't know it yet
    if (!m_deviceCache.contains(udi)) {
        m_deviceCache.append(udi);
        Q_EMIT deviceAdded(udi);
    }
    // re-emit in case of 2-stage devices like N9 or some Android phones
    else if (m_deviceCache.contains(udi) && interfaces_and_properties.keys().contains(UD2_DBUS_INTERFACE_FILESYSTEM)) {
        Q_EMIT deviceAdded(udi);
    }
}

void Manager::slotInterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces)
{
    const QString udi = object_path.path();
    if (udi.isEmpty()) {
        return;
    }

    /* Ignore jobs */
    if (udi.startsWith(UD2_DBUS_PATH_JOBS)) {
        return;
    }

    qCDebug(UDISKS2) << udi << "lost interfaces:" << interfaces;

    /*
     * Determine left interfaces. The device backend may have processed the
     * InterfacesRemoved signal already, but the result set is the same
     * independent if the backend or the manager processes the signal first.
     */
    Device device(udi);
    const QStringList ifaceList = device.interfaces();
    QSet<QString> leftInterfaces(ifaceList.begin(), ifaceList.end());
    leftInterfaces.subtract(QSet<QString>(interfaces.begin(), interfaces.end()));

    if (leftInterfaces.isEmpty()) {
        // remove the device if the last interface is removed
        Q_EMIT deviceRemoved(udi);
        m_deviceCache.removeAll(udi);
        DeviceBackend::destroyBackend(udi);
    } else {
        /*
         * Changes in the interface composition may change if a device
         * matches a Predicate. We have to do a remove-and-readd cycle
         * as there is no dedicated signal for Predicate reevaluation.
         */
        Q_EMIT deviceRemoved(udi);
        Q_EMIT deviceAdded(udi);
    }
}

void Manager::slotMediaChanged(const QDBusMessage &msg)
{
    const QVariantMap properties = qdbus_cast<QVariantMap>(msg.arguments().at(1));

    if (!properties.contains("Size")) { // react only on Size changes
        return;
    }

    const QString udi = msg.path();
    updateBackend(udi);
    qulonglong size = properties.value("Size").toULongLong();
    qCDebug(UDISKS2) << "MEDIA CHANGED in" << udi << "; size is:" << size;

    if (!m_deviceCache.contains(udi) && size > 0) { // we don't know the optdisc, got inserted
        m_deviceCache.append(udi);
        Q_EMIT deviceAdded(udi);
    }

    if (m_deviceCache.contains(udi) && size == 0) { // we know the optdisc, got removed
        Q_EMIT deviceRemoved(udi);
        m_deviceCache.removeAll(udi);
        DeviceBackend::destroyBackend(udi);
    }
}

const QStringList &Manager::deviceCache()
{
    if (m_deviceCache.isEmpty()) {
        allDevices();
    }

    return m_deviceCache;
}

void Manager::updateBackend(const QString &udi)
{
    DeviceBackend *backend = DeviceBackend::backendForUDI(udi);
    if (!backend) {
        return;
    }

    // This doesn't emit "changed" signals. Signals are emitted later by DeviceBackend's slots
    backend->allProperties();

    QVariant driveProp = backend->prop("Drive");
    if (!driveProp.isValid()) {
        return;
    }

    QDBusObjectPath drivePath = qdbus_cast<QDBusObjectPath>(driveProp);
    DeviceBackend *driveBackend = DeviceBackend::backendForUDI(drivePath.path(), false);
    if (!driveBackend) {
        return;
    }

    driveBackend->invalidateProperties();
}

#include "moc_udisksmanager.cpp"
