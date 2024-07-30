/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_FAKE_MODEM_OBJECT_MANAGER_H
#define MODEMMANAGERQT_FAKE_MODEM_OBJECT_MANAGER_H

#include "generictypes.h"
#include "generictypes_p.h"

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

class ObjectManager : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.DBus.ObjectManager")
public:
    explicit ObjectManager(QObject *parent = nullptr);
    ~ObjectManager() override;

    void addInterfaces(const QDBusObjectPath &object_path, const ModemManager::MMVariantMapMap &interfaces_and_properties);
    void removeInterfaces(const QDBusObjectPath &object_path, const QStringList &interfaces);
public Q_SLOTS:
    Q_SCRIPTABLE ModemManager::DBUSManagerStruct GetManagedObjects();

Q_SIGNALS:
    Q_SCRIPTABLE void InterfacesAdded(const QDBusObjectPath &object_path, const ModemManager::MMVariantMapMap &interfaces_and_properties);
    Q_SCRIPTABLE void InterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces);

private:
    QMap<QDBusObjectPath, ModemManager::MMVariantMapMap> m_managedObjects;
};

#endif
