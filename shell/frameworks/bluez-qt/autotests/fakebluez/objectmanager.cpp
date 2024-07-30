/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "objectmanager.h"
#include "object.h"

#include <QDBusConnection>
#include <QDBusMetaType>

ObjectManager *s_instance = nullptr;

ObjectManager::ObjectManager(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    s_instance = this;

    qDBusRegisterMetaType<QVariantMapMap>();
    qDBusRegisterMetaType<DBusManagerStruct>();

    QDBusConnection::sessionBus().registerObject(QStringLiteral("/org/bluez"), this);
}

ObjectManager::~ObjectManager()
{
    s_instance = nullptr;
    qDeleteAll(m_autoDeleteObjects);
}

void ObjectManager::addObject(Object *object)
{
    m_objects.insert(object->path().path(), object);

    QVariantMapMap interfaces;
    interfaces.insert(object->name(), object->properties());
    Q_EMIT InterfacesAdded(object->path(), interfaces);
}

void ObjectManager::removeObject(Object *object)
{
    m_objects.remove(object->path().path());
    m_autoDeleteObjects.removeOne(object->objectParent());

    Q_EMIT InterfacesRemoved(object->path(), QStringList(object->name()));

    delete object->objectParent();
}

void ObjectManager::addAutoDeleteObject(QObject *object)
{
    m_autoDeleteObjects.append(object);
}

Object *ObjectManager::objectByPath(const QDBusObjectPath &path) const
{
    return m_objects.value(path.path());
}

ObjectManager *ObjectManager::self()
{
    return s_instance;
}

DBusManagerStruct ObjectManager::GetManagedObjects()
{
    DBusManagerStruct objects;

    for (Object *object : m_objects.values()) {
        if (objects.value(object->path()).isEmpty()) {
            objects[object->path()].insert(QStringLiteral("org.freedesktop.DBus.Introspectable"), QVariantMap());
            if (object->haveProperties()) {
                objects[object->path()].insert(QStringLiteral("org.freedesktop.DBus.Properties"), QVariantMap());
            }
        }
        objects[object->path()].insert(object->name(), object->properties());
    }

    return objects;
}

#include "moc_objectmanager.cpp"
