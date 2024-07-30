/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "objectmanager.h"
#include "dbus/fakedbus.h"

#include <QDBusMetaType>

ObjectManager::ObjectManager(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    qDBusRegisterMetaType<QDBusObjectPath>();
    qDBusRegisterMetaType<ModemManager::MMVariantMapMap>();
    qDBusRegisterMetaType<ModemManager::DBUSManagerStruct>();
}

ObjectManager::~ObjectManager()
{
}

void ObjectManager::addInterfaces(const QDBusObjectPath &object_path, const ModemManager::MMVariantMapMap &interfaces_and_properties)
{
    if (m_managedObjects.contains(object_path)) {
        ModemManager::MMVariantMapMap map = m_managedObjects.value(object_path);
        map.insert(interfaces_and_properties);
        m_managedObjects.insert(object_path, map);
    } else {
        m_managedObjects.insert(object_path, interfaces_and_properties);
    }

    Q_EMIT InterfacesAdded(object_path, interfaces_and_properties);
}

void ObjectManager::removeInterfaces(const QDBusObjectPath &object_path, const QStringList &interfaces)
{
    if (interfaces.contains(QLatin1String(MMQT_DBUS_INTERFACE_MODEM))) {
        m_managedObjects.remove(object_path);
    } else {
        ModemManager::MMVariantMapMap map = m_managedObjects.value(object_path);
        Q_FOREACH (const QString &key, interfaces) {
            map.remove(key);
        }
        m_managedObjects.insert(object_path, map);
    }

    Q_EMIT InterfacesRemoved(object_path, interfaces);
}

ModemManager::DBUSManagerStruct ObjectManager::GetManagedObjects()
{
    return m_managedObjects;
}

#include "moc_objectmanager.cpp"
