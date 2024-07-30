/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_MANAGER_P_H
#define BLUEZQT_MANAGER_P_H

#include <QDBusContext>
#include <QHash>
#include <QObject>

#include "bluezagentmanager1.h"
#include "bluezprofilemanager1.h"
#include "dbusobjectmanager.h"
#include "rfkill.h"
#include "types.h"

namespace BluezQt
{
typedef org::freedesktop::DBus::ObjectManager DBusObjectManager;
typedef org::bluez::AgentManager1 BluezAgentManager;
typedef org::bluez::ProfileManager1 BluezProfileManager;

class Manager;
class Adapter;
class Device;
class AdapterPrivate;

class ManagerPrivate : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    explicit ManagerPrivate(Manager *parent);

    void init();
    void nameHasOwnerFinished(QDBusPendingCallWatcher *watcher);
    void load();
    void getManagedObjectsFinished(QDBusPendingCallWatcher *watcher);
    void clear();

    AdapterPtr findUsableAdapter() const;

    void serviceRegistered();
    void serviceUnregistered();
    void interfacesAdded(const QDBusObjectPath &objectPath, const QVariantMapMap &interfaces);
    void interfacesRemoved(const QDBusObjectPath &objectPath, const QStringList &interfaces);
    void adapterRemoved(const AdapterPtr &adapter);
    void adapterPoweredChanged(bool powered);
    void rfkillStateChanged(Rfkill::State state);

    void addAdapter(const QString &adapterPath, const QVariantMap &properties);
    void addDevice(const QString &devicePath, const QVariantMap &properties);
    void removeAdapter(const QString &adapterPath);
    void removeDevice(const QString &devicePath);

    bool rfkillBlocked() const;
    void setUsableAdapter(const AdapterPtr &adapter);

    Manager *q;
    Rfkill *m_rfkill;
    DBusObjectManager *m_dbusObjectManager;
    BluezAgentManager *m_bluezAgentManager;
    BluezProfileManager *m_bluezProfileManager;

    QHash<QString, AdapterPtr> m_adapters;
    QHash<QString, DevicePtr> m_devices;
    AdapterPtr m_usableAdapter;

    bool m_initialized;
    bool m_bluezRunning;
    bool m_loaded;
    bool m_adaptersLoaded;
    bool m_bluetoothBlocked;

Q_SIGNALS:
    void initError(const QString &errorText);
    void initFinished();

private Q_SLOTS:
    void propertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated);
    void dummy();
};

} // namespace BluezQt

#endif // BLUEZQT_MANAGER_P_H
