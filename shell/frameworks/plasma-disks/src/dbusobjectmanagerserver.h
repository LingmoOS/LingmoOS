// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#pragma once

#include <QChildEvent>
#include <QDBusObjectPath>
#include <QDebug>
#include <QMetaObject>
#include <QMetaProperty>

// Be very mindful when changing anything here. QMetaType<->DBus is very finicky.
typedef QMap<QString, QVariant> KDBusObjectManagerPropertiesMap;
typedef QMap<QString, KDBusObjectManagerPropertiesMap> KDBusObjectManagerInterfacePropertiesMap;
typedef QMap<QDBusObjectPath, KDBusObjectManagerInterfacePropertiesMap> KDBusObjectManagerObjectPathInterfacePropertiesMap;
typedef QStringList KDBusObjectManagerInterfaceList;

Q_DECLARE_METATYPE(KDBusObjectManagerPropertiesMap);
Q_DECLARE_METATYPE(KDBusObjectManagerInterfacePropertiesMap);
Q_DECLARE_METATYPE(KDBusObjectManagerObjectPathInterfacePropertiesMap);
Q_DECLARE_METATYPE(KDBusObjectManagerInterfaceList);

class KDBusObjectManagerServer : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.DBus.ObjectManager")
    typedef QHash<QString, const QMetaObject *> InterfaceMetaObjectHash;

public:
    KDBusObjectManagerServer(QObject *parent = nullptr);
    bool serve(QObject *object);
    void unserve(QObject *object);

    static void registerTypes();

public Q_SLOTS:
    KDBusObjectManagerObjectPathInterfacePropertiesMap GetManagedObjects();

Q_SIGNALS:
    void InterfacesAdded(QDBusObjectPath path, KDBusObjectManagerInterfacePropertiesMap map);
    void InterfacesRemoved(QDBusObjectPath path, KDBusObjectManagerInterfaceList list);

protected:
    QDBusObjectPath path(const QObject *object);
    KDBusObjectManagerInterfacePropertiesMap interfacePropertiesMap(const QObject *child);

private:
    InterfaceMetaObjectHash metaObjectsFor(const QObject *object);

    const QString m_path = QStringLiteral("/modules/smart/devices");
    QList<QObject *> m_managedObjects;
};
