/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "manager_p.h"
#include "adapter.h"
#include "adapter_p.h"
#include "debug.h"
#include "device.h"
#include "device_p.h"
#include "manager.h"
#include "utils.h"

#include <QDBusServiceWatcher>

namespace BluezQt
{
ManagerPrivate::ManagerPrivate(Manager *parent)
    : QObject(parent)
    , q(parent)
    , m_dbusObjectManager(nullptr)
    , m_bluezAgentManager(nullptr)
    , m_bluezProfileManager(nullptr)
    , m_initialized(false)
    , m_bluezRunning(false)
    , m_loaded(false)
    , m_adaptersLoaded(false)
{
    qDBusRegisterMetaType<DBusManagerStruct>();
    qRegisterMetaType<QVariantMapMap>("QVariantMapMap");
    qDBusRegisterMetaType<QVariantMapMap>();

    m_rfkill = new Rfkill(this);
    m_bluetoothBlocked = rfkillBlocked();
    connect(m_rfkill, &Rfkill::stateChanged, this, &ManagerPrivate::rfkillStateChanged);

    connect(q, &Manager::adapterRemoved, this, &ManagerPrivate::adapterRemoved);
}

void ManagerPrivate::init()
{
    // Keep an eye on org.bluez service
    QDBusServiceWatcher *serviceWatcher = new QDBusServiceWatcher(Strings::orgBluez(),
                                                                  DBusConnection::orgBluez(),
                                                                  QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration,
                                                                  this);

    connect(serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this, &ManagerPrivate::serviceRegistered);
    connect(serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, &ManagerPrivate::serviceUnregistered);

    // Update the current state of org.bluez service
    if (!DBusConnection::orgBluez().isConnected()) {
        Q_EMIT initError(QStringLiteral("DBus system bus is not connected!"));
        return;
    }

    QDBusMessage call =
        QDBusMessage::createMethodCall(Strings::orgFreedesktopDBus(), QStringLiteral("/"), Strings::orgFreedesktopDBus(), QStringLiteral("NameHasOwner"));

    call << Strings::orgBluez();

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(DBusConnection::orgBluez().asyncCall(call));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ManagerPrivate::nameHasOwnerFinished);

    DBusConnection::orgBluez().connect(Strings::orgBluez(),
                                       QString(),
                                       Strings::orgFreedesktopDBusProperties(),
                                       QStringLiteral("PropertiesChanged"),
                                       this,
                                       SLOT(propertiesChanged(QString, QVariantMap, QStringList)));
}

void ManagerPrivate::nameHasOwnerFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<bool> &reply = *watcher;
    watcher->deleteLater();

    if (reply.isError()) {
        Q_EMIT initError(reply.error().message());
        return;
    }

    m_bluezRunning = reply.value();

    if (m_bluezRunning) {
        load();
    } else {
        m_initialized = true;
        Q_EMIT initFinished();
    }
}

void ManagerPrivate::load()
{
    if (!m_bluezRunning || m_loaded) {
        return;
    }

    // Force QDBus to cache owner of org.bluez - this will be the only blocking call on system connection
    DBusConnection::orgBluez().connect(Strings::orgBluez(), QStringLiteral("/"), Strings::orgFreedesktopDBus(), QStringLiteral("Dummy"), this, SLOT(dummy()));

    m_dbusObjectManager = new DBusObjectManager(Strings::orgBluez(), QStringLiteral("/"), DBusConnection::orgBluez(), this);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(m_dbusObjectManager->GetManagedObjects(), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ManagerPrivate::getManagedObjectsFinished);
}

void ManagerPrivate::getManagedObjectsFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<DBusManagerStruct> &reply = *watcher;
    watcher->deleteLater();

    if (reply.isError()) {
        Q_EMIT initError(reply.error().message());
        return;
    }

    DBusManagerStruct::const_iterator it;
    const DBusManagerStruct &managedObjects = reply.value();

    for (it = managedObjects.constBegin(); it != managedObjects.constEnd(); ++it) {
        const QString &path = it.key().path();
        const QVariantMapMap &interfaces = it.value();

        interfacesAdded(it.key(), interfaces);

        if (interfaces.contains(Strings::orgBluezAgentManager1())) {
            m_bluezAgentManager = new BluezAgentManager(Strings::orgBluez(), path, DBusConnection::orgBluez(), this);
        }
        if (interfaces.contains(Strings::orgBluezProfileManager1())) {
            m_bluezProfileManager = new BluezProfileManager(Strings::orgBluez(), path, DBusConnection::orgBluez(), this);
        }
    }

    if (!m_bluezAgentManager) {
        Q_EMIT initError(QStringLiteral("Cannot find org.bluez.AgentManager1 object!"));
        return;
    }

    if (!m_bluezProfileManager) {
        Q_EMIT initError(QStringLiteral("Cannot find org.bluez.ProfileManager1 object!"));
        return;
    }

    connect(m_dbusObjectManager, &DBusObjectManager::InterfacesAdded, this, &ManagerPrivate::interfacesAdded);
    connect(m_dbusObjectManager, &DBusObjectManager::InterfacesRemoved, this, &ManagerPrivate::interfacesRemoved);

    m_loaded = true;
    m_initialized = true;

    Q_EMIT q->operationalChanged(true);

    if (q->isBluetoothOperational()) {
        Q_EMIT q->bluetoothOperationalChanged(true);
    }

    Q_EMIT initFinished();
}

void ManagerPrivate::clear()
{
    m_loaded = false;

    // Delete all devices first
    while (!m_devices.isEmpty()) {
        DevicePtr device = m_devices.begin().value();
        m_devices.remove(m_devices.begin().key());
        device->adapter()->d->removeDevice(device);
    }

    // Delete all adapters
    while (!m_adapters.isEmpty()) {
        AdapterPtr adapter = m_adapters.begin().value();
        m_adapters.remove(m_adapters.begin().key());
        Q_EMIT adapter->adapterRemoved(adapter);

        if (m_adapters.isEmpty()) {
            Q_EMIT q->allAdaptersRemoved();
        }
    }

    // Delete all other objects
    m_usableAdapter.clear();

    if (m_dbusObjectManager) {
        m_dbusObjectManager->deleteLater();
        m_dbusObjectManager = nullptr;
    }

    if (m_bluezAgentManager) {
        m_bluezAgentManager->deleteLater();
        m_bluezAgentManager = nullptr;
    }
}

AdapterPtr ManagerPrivate::findUsableAdapter() const
{
    for (AdapterPtr adapter : std::as_const(m_adapters)) {
        if (adapter->isPowered()) {
            return adapter;
        }
    }
    return AdapterPtr();
}

void ManagerPrivate::serviceRegistered()
{
    qCDebug(BLUEZQT) << "BlueZ service registered";
    m_bluezRunning = true;

    load();
}

void ManagerPrivate::serviceUnregistered()
{
    qCDebug(BLUEZQT) << "BlueZ service unregistered";

    bool wasBtOperational = q->isBluetoothOperational();
    m_bluezRunning = false;

    if (wasBtOperational) {
        Q_EMIT q->bluetoothOperationalChanged(false);
    }

    clear();
    Q_EMIT q->operationalChanged(false);
}

void ManagerPrivate::interfacesAdded(const QDBusObjectPath &objectPath, const QVariantMapMap &interfaces)
{
    const QString &path = objectPath.path();
    QVariantMapMap::const_iterator it;

    for (it = interfaces.constBegin(); it != interfaces.constEnd(); ++it) {
        if (it.key() == Strings::orgBluezAdapter1()) {
            addAdapter(path, it.value());
        } else if (it.key() == Strings::orgBluezDevice1()) {
            addDevice(path, it.value());
        }
    }

    for (auto it = m_adapters.cbegin(); it != m_adapters.cend(); ++it) {
        if (path.startsWith(it.value()->ubi())) {
            it.value()->d->interfacesAdded(path, interfaces);
            break;
        }
    }

    for (auto it = m_devices.cbegin(); it != m_devices.cend(); ++it) {
        if (path.startsWith(it.value()->ubi())) {
            it.value()->d->interfacesAdded(path, interfaces);
            break;
        }
    }
}

void ManagerPrivate::interfacesRemoved(const QDBusObjectPath &objectPath, const QStringList &interfaces)
{
    const QString &path = objectPath.path();

    for (const QString &interface : interfaces) {
        if (interface == Strings::orgBluezAdapter1()) {
            removeAdapter(path);
        } else if (interface == Strings::orgBluezDevice1()) {
            removeDevice(path);
        }
    }

    for (auto it = m_adapters.cbegin(); it != m_adapters.cend(); ++it) {
        if (path.startsWith(it.value()->ubi())) {
            it.value()->d->interfacesRemoved(path, interfaces);
            break;
        }
    }

    for (auto it = m_devices.cbegin(); it != m_devices.cend(); ++it) {
        if (path.startsWith(it.value()->ubi())) {
            it.value()->d->interfacesRemoved(path, interfaces);
            break;
        }
    }
}

void ManagerPrivate::adapterRemoved(const AdapterPtr &adapter)
{
    disconnect(adapter.data(), &Adapter::poweredChanged, this, &ManagerPrivate::adapterPoweredChanged);

    // Current usable adapter was removed
    if (adapter == m_usableAdapter) {
        setUsableAdapter(findUsableAdapter());
    }
}

void ManagerPrivate::adapterPoweredChanged(bool powered)
{
    Q_ASSERT(qobject_cast<Adapter *>(sender()));
    AdapterPtr adapter = static_cast<Adapter *>(sender())->toSharedPtr();

    // Current usable adapter was powered off
    if (m_usableAdapter == adapter && !powered) {
        setUsableAdapter(findUsableAdapter());
    }

    // Adapter was powered on, set it as usable
    if (!m_usableAdapter && powered) {
        setUsableAdapter(adapter);
    }
}

void ManagerPrivate::rfkillStateChanged(Rfkill::State state)
{
    Q_UNUSED(state)

    bool blocked = rfkillBlocked();
    bool wasBtOperational = q->isBluetoothOperational();

    if (m_bluetoothBlocked != blocked) {
        m_bluetoothBlocked = blocked;
        Q_EMIT q->bluetoothBlockedChanged(m_bluetoothBlocked);
        if (wasBtOperational != q->isBluetoothOperational()) {
            Q_EMIT q->bluetoothOperationalChanged(q->isBluetoothOperational());
        }
    }
}

void ManagerPrivate::addAdapter(const QString &adapterPath, const QVariantMap &properties)
{
    AdapterPtr adapter = AdapterPtr(new Adapter(adapterPath, properties));
    adapter->d->q = adapter.toWeakRef();
    m_adapters.insert(adapterPath, adapter);

    Q_EMIT q->adapterAdded(adapter);

    // Powered adapter was added, set it as usable
    if (!m_usableAdapter && adapter->isPowered()) {
        setUsableAdapter(adapter);
    }

    connect(adapter.data(), &Adapter::deviceAdded, q, &Manager::deviceAdded);
    connect(adapter.data(), &Adapter::adapterRemoved, q, &Manager::adapterRemoved);
    connect(adapter.data(), &Adapter::adapterChanged, q, &Manager::adapterChanged);
    connect(adapter.data(), &Adapter::poweredChanged, this, &ManagerPrivate::adapterPoweredChanged);
}

void ManagerPrivate::addDevice(const QString &devicePath, const QVariantMap &properties)
{
    AdapterPtr adapter = m_adapters.value(properties.value(QStringLiteral("Adapter")).value<QDBusObjectPath>().path());
    if (!adapter) {
        return;
    }

    DevicePtr device = DevicePtr(new Device(devicePath, properties, adapter));
    device->d->q = device.toWeakRef();
    m_devices.insert(devicePath, device);
    adapter->d->addDevice(device);

    connect(device.data(), &Device::deviceRemoved, q, &Manager::deviceRemoved);
    connect(device.data(), &Device::deviceChanged, q, &Manager::deviceChanged);
}

void ManagerPrivate::removeAdapter(const QString &adapterPath)
{
    AdapterPtr adapter = m_adapters.value(adapterPath);
    if (!adapter) {
        return;
    }

    // Make sure we always remove all devices before removing the adapter
    const auto devices = adapter->devices();
    for (const DevicePtr &device : devices) {
        removeDevice(device->ubi());
    }

    m_adapters.remove(adapterPath);
    Q_EMIT adapter->adapterRemoved(adapter);

    if (m_adapters.isEmpty()) {
        Q_EMIT q->allAdaptersRemoved();
    }

    disconnect(adapter.data(), &Adapter::adapterChanged, q, &Manager::adapterChanged);
    disconnect(adapter.data(), &Adapter::poweredChanged, this, &ManagerPrivate::adapterPoweredChanged);
}

void ManagerPrivate::removeDevice(const QString &devicePath)
{
    DevicePtr device = m_devices.take(devicePath);
    if (!device) {
        return;
    }

    device->adapter()->d->removeDevice(device);

    disconnect(device.data(), &Device::deviceChanged, q, &Manager::deviceChanged);
}

bool ManagerPrivate::rfkillBlocked() const
{
    return m_rfkill->state() == Rfkill::SoftBlocked || m_rfkill->state() == Rfkill::HardBlocked;
}

void ManagerPrivate::setUsableAdapter(const AdapterPtr &adapter)
{
    if (m_usableAdapter == adapter) {
        return;
    }

    qCDebug(BLUEZQT) << "Setting usable adapter" << adapter;

    bool wasBtOperational = q->isBluetoothOperational();

    m_usableAdapter = adapter;
    Q_EMIT q->usableAdapterChanged(m_usableAdapter);

    if (wasBtOperational != q->isBluetoothOperational()) {
        Q_EMIT q->bluetoothOperationalChanged(q->isBluetoothOperational());
    }
}

void ManagerPrivate::propertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated)
{
    // Cut anything after device path to forward it to Device to handle
    const QString path_full = message().path();
    const QString path_device = path_full.section(QLatin1Char('/'), 0, 4);

    QTimer::singleShot(0, this, [this, interface, changed, invalidated, path_full, path_device]() {
        AdapterPtr adapter = m_adapters.value(path_device);
        if (adapter) {
            adapter->d->propertiesChanged(interface, changed, invalidated);
            return;
        }
        DevicePtr device = m_devices.value(path_device);
        if (device) {
            device->d->propertiesChanged(path_full, interface, changed, invalidated);
            return;
        }
        qCDebug(BLUEZQT) << "Unhandled property change" << interface << changed << invalidated;
    });
}

void ManagerPrivate::dummy()
{
}

} // namespace BluezQt

#include "moc_manager_p.cpp"
