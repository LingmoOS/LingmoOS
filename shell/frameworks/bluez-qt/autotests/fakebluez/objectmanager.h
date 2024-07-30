/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>
#include <QVariantMap>

typedef QMap<QString, QVariantMap> QVariantMapMap;
Q_DECLARE_METATYPE(QVariantMapMap)

typedef QMap<QDBusObjectPath, QVariantMapMap> DBusManagerStruct;
Q_DECLARE_METATYPE(DBusManagerStruct)

class Object;

class ObjectManager : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.DBus.ObjectManager")

public:
    explicit ObjectManager(QObject *parent = nullptr);
    ~ObjectManager() override;

    void addObject(Object *object);
    void removeObject(Object *object);

    void addAutoDeleteObject(QObject *object);

    Object *objectByPath(const QDBusObjectPath &path) const;

    static ObjectManager *self();

public Q_SLOTS:
    DBusManagerStruct GetManagedObjects();

Q_SIGNALS:
    void InterfacesAdded(const QDBusObjectPath &object, const QVariantMapMap &interfaces);
    void InterfacesRemoved(const QDBusObjectPath &object, const QStringList &interfaces);

private:
    QMultiMap<QString, Object *> m_objects;
    QList<QObject *> m_autoDeleteObjects;
};

#endif // OBJECTMANAGER_H
