/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "manager.h"

#include "adapter.h"
#include "agent.h"
#include "agentadaptor.h"
#include "debug.h"
#include "initmanagerjob.h"
#include "manager_p.h"
#include "pendingcall.h"
#include "profile.h"
#include "profile_p.h"
#include "profileadaptor.h"
#include "utils.h"

namespace BluezQt
{
Manager::Manager(QObject *parent)
    : QObject(parent)
    , d(new ManagerPrivate(this))
{
    Instance::setManager(this);
}

Manager::~Manager() = default;

InitManagerJob *Manager::init()
{
    return new InitManagerJob(this);
}

bool Manager::isInitialized() const
{
    return d->m_initialized;
}

bool Manager::isOperational() const
{
    return d->m_initialized && d->m_bluezRunning && d->m_loaded;
}

bool Manager::isBluetoothOperational() const
{
    return !d->m_bluetoothBlocked && d->m_bluezRunning && d->m_loaded && d->m_usableAdapter;
}

bool Manager::isBluetoothBlocked() const
{
    return d->m_bluetoothBlocked;
}

void Manager::setBluetoothBlocked(bool blocked)
{
    if (blocked) {
        d->m_rfkill->block();
    } else {
        d->m_rfkill->unblock();
    }
}

AdapterPtr Manager::usableAdapter() const
{
    return d->m_usableAdapter;
}

QList<AdapterPtr> Manager::adapters() const
{
    return d->m_adapters.values();
}

QList<DevicePtr> Manager::devices() const
{
    return d->m_devices.values();
}

PendingCall *Manager::startService()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(Strings::orgFreedesktopDBus(),
                                                      QStringLiteral("/org/freedesktop/DBus"),
                                                      Strings::orgFreedesktopDBus(),
                                                      QStringLiteral("StartServiceByName"));
    msg << Strings::orgBluez();
    msg << quint32(0);

    return new PendingCall(DBusConnection::orgBluez().asyncCall(msg), PendingCall::ReturnUint32);
}

AdapterPtr Manager::adapterForAddress(const QString &address) const
{
    for (AdapterPtr adapter : std::as_const(d->m_adapters)) {
        if (adapter->address() == address) {
            return adapter;
        }
    }
    return AdapterPtr();
}

AdapterPtr Manager::adapterForUbi(const QString &ubi) const
{
    return d->m_adapters.value(ubi);
}

DevicePtr Manager::deviceForAddress(const QString &address) const
{
    DevicePtr device;

    for (AdapterPtr adapter : std::as_const(d->m_adapters)) {
        DevicePtr oneDevice = adapter->deviceForAddress(address);
        if (!oneDevice) {
            continue;
        }

        // Prefer powered adapter
        if (!device) {
            device = oneDevice;
        } else if (adapter->isPowered()) {
            device = oneDevice;
        }
    }

    return device;
}

DevicePtr Manager::deviceForUbi(const QString &ubi) const
{
    return d->m_devices.value(ubi);
}

PendingCall *Manager::registerAgent(Agent *agent)
{
    Q_ASSERT(agent);

    if (!d->m_bluezAgentManager) {
        return new PendingCall(PendingCall::InternalError, QStringLiteral("Manager not operational!"));
    }

    QString capability;

    switch (agent->capability()) {
    case Agent::DisplayOnly:
        capability = QStringLiteral("DisplayOnly");
        break;
    case Agent::DisplayYesNo:
        capability = QStringLiteral("DisplayYesNo");
        break;
    case Agent::KeyboardOnly:
        capability = QStringLiteral("KeyboardOnly");
        break;
    case Agent::NoInputNoOutput:
        capability = QStringLiteral("NoInputNoOutput");
        break;
    default:
        capability = QStringLiteral("DisplayYesNo");
        break;
    }

    new AgentAdaptor(agent, this);

    if (!DBusConnection::orgBluez().registerObject(agent->objectPath().path(), agent)) {
        qCDebug(BLUEZQT) << "Cannot register object" << agent->objectPath().path();
    }

    return new PendingCall(d->m_bluezAgentManager->RegisterAgent(agent->objectPath(), capability), PendingCall::ReturnVoid, this);
}

PendingCall *Manager::unregisterAgent(Agent *agent)
{
    Q_ASSERT(agent);

    if (!d->m_bluezAgentManager) {
        return new PendingCall(PendingCall::InternalError, QStringLiteral("Manager not operational!"));
    }

    DBusConnection::orgBluez().unregisterObject(agent->objectPath().path());

    return new PendingCall(d->m_bluezAgentManager->UnregisterAgent(agent->objectPath()), PendingCall::ReturnVoid, this);
}

PendingCall *Manager::requestDefaultAgent(Agent *agent)
{
    Q_ASSERT(agent);

    if (!d->m_bluezAgentManager) {
        return new PendingCall(PendingCall::InternalError, QStringLiteral("Manager not operational!"));
    }

    return new PendingCall(d->m_bluezAgentManager->RequestDefaultAgent(agent->objectPath()), PendingCall::ReturnVoid, this);
}

PendingCall *Manager::registerProfile(Profile *profile)
{
    Q_ASSERT(profile);

    if (!d->m_bluezProfileManager) {
        return new PendingCall(PendingCall::InternalError, QStringLiteral("Manager not operational!"));
    }

    new ProfileAdaptor(profile, this);

    if (!DBusConnection::orgBluez().registerObject(profile->objectPath().path(), profile)) {
        qCDebug(BLUEZQT) << "Cannot register object" << profile->objectPath().path();
    }

    return new PendingCall(d->m_bluezProfileManager->RegisterProfile(profile->objectPath(), profile->uuid(), profile->d->options),
                           PendingCall::ReturnVoid,
                           this);
}

PendingCall *Manager::unregisterProfile(Profile *profile)
{
    Q_ASSERT(profile);

    if (!d->m_bluezProfileManager) {
        return new PendingCall(PendingCall::InternalError, QStringLiteral("Manager not operational!"));
    }

    DBusConnection::orgBluez().unregisterObject(profile->objectPath().path());

    return new PendingCall(d->m_bluezProfileManager->UnregisterProfile(profile->objectPath()), PendingCall::ReturnVoid, this);
}

Rfkill *Manager::rfkill() const
{
    return d->m_rfkill;
}

} // namespace BluezQt

#include "moc_manager.cpp"
