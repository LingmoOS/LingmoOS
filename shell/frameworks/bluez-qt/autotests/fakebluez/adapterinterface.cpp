/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "adapterinterface.h"
#include "gattmanagerinterface.h"
#include "leadvertisingmanagerinterface.h"
#include "mediainterface.h"
#include "objectmanager.h"

#include <QDBusConnection>
#include <QTimer>

// AdapterObject
AdapterObject::AdapterObject(const QDBusObjectPath &path, QObject *parent)
    : QObject(parent)
{
    QDBusConnection::sessionBus().registerObject(path.path(), this);
}

// AdapterInterface
AdapterInterface::AdapterInterface(const QDBusObjectPath &path, const QVariantMap &properties, QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setPath(path);
    setObjectParent(parent);
    setProperties(properties);
    setName(QStringLiteral("org.bluez.Adapter1"));

    // Alias needs special handling
    setAlias(properties.value(QStringLiteral("Alias")).toString());

    // Media interface
    m_media = new MediaInterface(path, parent);
    ObjectManager *manager = ObjectManager::self();
    manager->addObject(m_media);

    // LEAdvertisingManager interface
    m_leAdvertisingManager = new LEAdvertisingManagerInterface(path, parent);
    manager->addObject(m_leAdvertisingManager);

    // GattManager interface
    m_gattManager = new GattManagerInterface(path, parent);
    manager->addObject(m_gattManager);
}

QString AdapterInterface::address() const
{
    return Object::property(QStringLiteral("Address")).toString();
}

QString AdapterInterface::name() const
{
    return Object::property(QStringLiteral("Name")).toString();
}

QString AdapterInterface::alias() const
{
    return Object::property(QStringLiteral("Alias")).toString();
}

void AdapterInterface::setAlias(const QString &alias)
{
    Object::changeProperty(QStringLiteral("Alias"), alias.isEmpty() ? name() : alias);
}

quint32 AdapterInterface::adapterClass() const
{
    return Object::property(QStringLiteral("Class")).toUInt();
}

bool AdapterInterface::powered() const
{
    return Object::property(QStringLiteral("Powered")).toBool();
}

void AdapterInterface::setPowered(bool powered)
{
    Object::changeProperty(QStringLiteral("Powered"), powered);
}

bool AdapterInterface::discoverable() const
{
    return Object::property(QStringLiteral("Discoverable")).toBool();
}

void AdapterInterface::setDiscoverable(bool discoverable)
{
    Object::changeProperty(QStringLiteral("Discoverable"), discoverable);

    if (discoverable && discoverableTimeout() != 0) {
        QTimer::singleShot(discoverableTimeout() * 1000, this, SLOT(resetDiscoverable()));
    }
}

bool AdapterInterface::pairable() const
{
    return Object::property(QStringLiteral("Pairable")).toBool();
}

void AdapterInterface::setPairable(bool pairable)
{
    Object::changeProperty(QStringLiteral("Pairable"), pairable);

    if (pairable && pairableTimeout() != 0) {
        QTimer::singleShot(pairableTimeout() * 1000, this, SLOT(resetPairable()));
    }
}

quint32 AdapterInterface::pairableTimeout() const
{
    return Object::property(QStringLiteral("PairableTimeout")).toUInt();
}

void AdapterInterface::setPairableTimeout(quint32 timeout)
{
    Object::changeProperty(QStringLiteral("PairableTimeout"), timeout);
}

quint32 AdapterInterface::discoverableTimeout() const
{
    return Object::property(QStringLiteral("DiscoverableTimeout")).toUInt();
}

void AdapterInterface::setDiscoverableTimeout(quint32 timeout)
{
    Object::changeProperty(QStringLiteral("DiscoverableTimeout"), timeout);
}

bool AdapterInterface::discovering() const
{
    return Object::property(QStringLiteral("Discovering")).toBool();
}

QStringList AdapterInterface::uuids() const
{
    return Object::property(QStringLiteral("UUIDs")).toStringList();
}

QString AdapterInterface::modalias() const
{
    return Object::property(QStringLiteral("Modalias")).toString();
}

MediaInterface *AdapterInterface::media() const
{
    return m_media;
}

LEAdvertisingManagerInterface *AdapterInterface::leAdvertisingManager() const
{
    return m_leAdvertisingManager;
}

GattManagerInterface *AdapterInterface::gattManager() const
{
    return m_gattManager;
}

void AdapterInterface::StartDiscovery()
{
    Object::changeProperty(QStringLiteral("Discovering"), true);
}

void AdapterInterface::StopDiscovery()
{
    Object::changeProperty(QStringLiteral("Discovering"), false);
}

void AdapterInterface::RemoveDevice(const QDBusObjectPath &device)
{
    ObjectManager *manager = ObjectManager::self();
    manager->removeObject(manager->objectByPath(device));
}

void AdapterInterface::SetDiscoveryFilter(const QVariantMap &filter)
{
    // Bluez makes adapter discoverable while discovering if 'Discoverable' option is set
    if (filter.contains(QStringLiteral("Discoverable")) && filter.value(QStringLiteral("Discoverable")).toBool())
    {
        Object::changeProperty(QStringLiteral("Discoverable"), true);
    }
}

QStringList AdapterInterface::GetDiscoveryFilters()
{
    return {
        QStringLiteral("UUIDs"),
        QStringLiteral("RSSI"),
        QStringLiteral("Pathloss"),
        QStringLiteral("Transport"),
        QStringLiteral("DuplicateData"),
        QStringLiteral("Discoverable"),
        QStringLiteral("Pattern")
    };
}

void AdapterInterface::resetPairable()
{
    setPairable(false);
}

void AdapterInterface::resetDiscoverable()
{
    setDiscoverable(false);
}

#include "moc_adapterinterface.cpp"
