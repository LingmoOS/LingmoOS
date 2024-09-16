// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dobjectmanagerinterface.h"
#include <QDebug>
#include <QDBusMetaType>

DBLUETOOTH_BEGIN_NAMESPACE

DObjectManagerInterface::DObjectManagerInterface(const QString &service, QObject *parent)
    : QObject(parent)
{
    qDBusRegisterMetaType<ObjectMap>();
    qRegisterMetaType<ObjectMap>("ObjectMap");
    qDBusRegisterMetaType<MapVariantMap>();
    qRegisterMetaType<MapVariantMap>("MapVariantMap");

#ifndef USE_FAKE_INTERFACE
    auto connection = QDBusConnection::systemBus();
#else
    auto connection = QDBusConnection::sessionBus();
#endif
    const auto &Path = QLatin1String("/");
    const auto &Interface = QLatin1String("org.freedesktop.DBus.ObjectManager");
    connection.connect(service,
                       Path,
                       Interface,
                       QLatin1String("InterfacesAdded"),
                       this,
                       SLOT(InterfacesAdd(const QDBusObjectPath &, const Interfaces &)));
    connection.connect(service,
                       Path,
                       Interface,
                       QLatin1String("InterfacesRemoved"),
                       this,
                       SLOT(InterfacesRemove(const QDBusObjectPath &, const QStringList &)));
    m_inter = new DDBusInterface(service, Path, Interface, connection, this);
}

QDBusPendingReply<ObjectMap> DObjectManagerInterface::getManagedObjects()
{
    return m_inter->asyncCall("GetManagedObjects");
}

void DObjectManagerInterface::InterfacesAdd(const QDBusObjectPath &path, const MapVariantMap &interfaces)
{
    Q_EMIT InterfacesAdded(path, interfaces);
}

void DObjectManagerInterface::InterfacesRemove(const QDBusObjectPath &path, const QStringList &args)
{
    Q_EMIT InterfacesRemoved(path, args);
}

DBLUETOOTH_END_NAMESPACE
