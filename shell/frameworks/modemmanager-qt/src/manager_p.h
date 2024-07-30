/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2010 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MANAGER_P_H
#define MODEMMANAGERQT_MANAGER_P_H

#include <QDBusServiceWatcher>

#include "dbus/dbus_manager.h"
#include "dbus/modem3gppinterface.h"
#include "dbus/modemmanager1interface.h"

#include "manager.h"
#include "modemdevice.h"

namespace ModemManager
{
class Modem;
class ModemManagerPrivate : public Notifier
{
    Q_OBJECT

public:
    ModemManagerPrivate();
    ~ModemManagerPrivate() override;
    QDBusServiceWatcher watcher;
    OrgFreedesktopModemManager1Interface iface;
    QMap<QString, ModemDevice::Ptr> modemList;
    OrgFreedesktopDBusObjectManagerInterface manager;
    ModemManager::ModemDevice::Ptr findModemDevice(const QString &uni);
    ModemManager::ModemDevice::List modemDevices();

    void scanDevices();
protected Q_SLOTS:
    void init();
    void daemonRegistered();
    void daemonUnregistered();
    void onInterfacesAdded(const QDBusObjectPath &object_path, const MMVariantMapMap &interfaces_and_properties);
    void onInterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces);
};
} // namespace ModemManager

#endif
