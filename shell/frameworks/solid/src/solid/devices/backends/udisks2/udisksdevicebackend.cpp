/*
    SPDX-FileCopyrightText: 2010 Michael Zanetti <mzanetti@kde.org>
    SPDX-FileCopyrightText: 2010-2012 Lukáš Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2012 Dan Vrátil <dvratil@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udisksdevicebackend.h"
#include "udisks_debug.h"

#include <QDBusConnection>
#include <QXmlStreamReader>

#include "solid/deviceinterface.h"
#include "solid/genericinterface.h"

using namespace Solid::Backends::UDisks2;

/* Static cache for DeviceBackends for all UDIs */
QThreadStorage<QMap<QString /* UDI */, DeviceBackend *>> DeviceBackend::s_backends;

DeviceBackend *DeviceBackend::backendForUDI(const QString &udi, bool create)
{
    DeviceBackend *backend = nullptr;
    if (udi.isEmpty()) {
        return backend;
    }

    backend = s_backends.localData().value(udi);
    if (!backend && create) {
        backend = new DeviceBackend(udi);
        s_backends.localData().insert(udi, backend);
    }

    return backend;
}

void DeviceBackend::destroyBackend(const QString &udi)
{
    delete s_backends.localData().take(udi);
}

DeviceBackend::DeviceBackend(const QString &udi)
    : m_udi(udi)
{
    // qDebug() << "Creating backend for device" << m_udi;

    QDBusConnection::systemBus().connect(UD2_DBUS_SERVICE, //
                                         m_udi,
                                         DBUS_INTERFACE_PROPS,
                                         "PropertiesChanged",
                                         this,
                                         SLOT(slotPropertiesChanged(QString, QVariantMap, QStringList)));
    QDBusConnection::systemBus().connect(UD2_DBUS_SERVICE,
                                         UD2_DBUS_PATH,
                                         DBUS_INTERFACE_MANAGER,
                                         "InterfacesAdded",
                                         this,
                                         SLOT(slotInterfacesAdded(QDBusObjectPath, VariantMapMap)));
    QDBusConnection::systemBus().connect(UD2_DBUS_SERVICE,
                                         UD2_DBUS_PATH,
                                         DBUS_INTERFACE_MANAGER,
                                         "InterfacesRemoved",
                                         this,
                                         SLOT(slotInterfacesRemoved(QDBusObjectPath, QStringList)));

    initInterfaces();
}

DeviceBackend::~DeviceBackend()
{
    // qDebug() << "Destroying backend for device" << m_udi;
}

void DeviceBackend::initInterfaces()
{
    m_interfaces.clear();

    const QString xmlData = introspect();
    if (xmlData.isEmpty()) {
        qCDebug(UDISKS2) << m_udi << "has no interfaces!";
        return;
    }

    QXmlStreamReader xml(xmlData);
    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == QLatin1String("interface")) {
            const auto name = xml.attributes().value(QLatin1String("name")).toString();
            /* Accept only org.freedesktop.UDisks2.* interfaces so that when the device is unplugged,
             * m_interfaces goes empty and we can easily verify that the device is gone. */
            if (name.startsWith(UD2_DBUS_SERVICE)) {
                m_interfaces.append(name);
            }
        }
    }

    // qDebug() << m_udi << "has interfaces:" << m_interfaces;
}

QStringList DeviceBackend::interfaces() const
{
    return m_interfaces;
}

const QString &DeviceBackend::udi() const
{
    return m_udi;
}

QVariant DeviceBackend::prop(const QString &key) const
{
    checkCache(key);
    return m_propertyCache.value(key);
}

bool DeviceBackend::propertyExists(const QString &key) const
{
    checkCache(key);
    /* checkCache() will put an invalid QVariant in cache when the property
     * does not exist, so check for validity, not for an actual presence. */
    return m_propertyCache.value(key).isValid();
}

QVariantMap DeviceBackend::allProperties() const
{
    QDBusMessage call = QDBusMessage::createMethodCall(UD2_DBUS_SERVICE, //
                                                       m_udi,
                                                       DBUS_INTERFACE_PROPS,
                                                       "GetAll");

    for (const QString &iface : std::as_const(m_interfaces)) {
        call.setArguments(QVariantList() << iface);
        QDBusPendingReply<QVariantMap> reply = QDBusConnection::systemBus().call(call);

        if (reply.isValid()) {
            auto props = reply.value();
            // Can not use QMap<>::unite(), as it allows multiple values per key
            for (auto it = props.cbegin(); it != props.cend(); ++it) {
                cacheProperty(it.key(), it.value());
            }
        } else {
            qCWarning(UDISKS2) << "Error getting props:" << reply.error().name() << reply.error().message() << "for" << m_udi;
        }
        // qDebug() << "After iface" << iface << ", cache now contains" << m_propertyCache.size() << "items";
    }

    return m_propertyCache;
}

void DeviceBackend::invalidateProperties()
{
    m_propertyCache.clear();
}

QString DeviceBackend::introspect() const
{
    QDBusMessage call = QDBusMessage::createMethodCall(UD2_DBUS_SERVICE, m_udi, DBUS_INTERFACE_INTROSPECT, "Introspect");
    QDBusPendingReply<QString> reply = QDBusConnection::systemBus().call(call);

    if (reply.isValid()) {
        return reply.value();
    } else {
        return QString();
    }
}

void DeviceBackend::checkCache(const QString &key) const
{
    if (m_propertyCache.isEmpty()) { // recreate the cache
        allProperties();
    }

    if (m_propertyCache.contains(key)) {
        return;
    }

    QDBusMessage call = QDBusMessage::createMethodCall(UD2_DBUS_SERVICE, m_udi, DBUS_INTERFACE_PROPS, "Get");
    /*
     * Interface is set to an empty string as in this QDBusInterface is a meta-object of multiple interfaces on the same path
     * The DBus properties also interface supports this, and will find the appropriate interface if none is explicitly set.
     * This matches what QDBusAbstractInterface would do
     */
    call.setArguments(QVariantList() << QString() << key);
    QDBusPendingReply<QVariant> reply = QDBusConnection::systemBus().call(call);

    /* We don't check for error here and store the item in the cache anyway so next time we don't have to
     * do the DBus call to find out it does not exist but just check whether
     * prop(key).isValid() */
    cacheProperty(key, reply.value());
}

void DeviceBackend::slotPropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps, const QStringList &invalidatedProps)
{
    if (!ifaceName.startsWith(UD2_DBUS_SERVICE)) {
        return;
    }
    // qDebug() << m_udi << "'s interface" << ifaceName << "changed props:";

    QMap<QString, int> changeMap;

    for (const QString &key : invalidatedProps) {
        m_propertyCache.remove(key);
        changeMap.insert(key, Solid::GenericInterface::PropertyModified);
        // qDebug() << "\t invalidated:" << key;
    }

    QMapIterator<QString, QVariant> i(changedProps);
    while (i.hasNext()) {
        i.next();
        const QString key = i.key();
        cacheProperty(key, i.value()); // replace the value
        changeMap.insert(key, Solid::GenericInterface::PropertyModified);
        // qDebug() << "\t modified:" << key << ":" << m_propertyCache.value(key);
    }

    Q_EMIT propertyChanged(changeMap);
    Q_EMIT changed();
}

void DeviceBackend::slotInterfacesAdded(const QDBusObjectPath &object_path, const VariantMapMap &interfaces_and_properties)
{
    if (object_path.path() != m_udi) {
        return;
    }

    for (auto it = interfaces_and_properties.cbegin(); it != interfaces_and_properties.cend(); ++it) {
        const QString &iface = it.key();
        /* Don't store generic DBus interfaces */
        if (iface.startsWith(UD2_DBUS_SERVICE)) {
            m_interfaces.append(iface);
        }
    }
}

void DeviceBackend::slotInterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces)
{
    if (object_path.path() != m_udi) {
        return;
    }

    for (const QString &iface : interfaces) {
        m_interfaces.removeAll(iface);
    }

    // We don't know which property belongs to which interface, so remove all
    m_propertyCache.clear();
    if (!m_interfaces.isEmpty()) {
        allProperties();
    }
}

// UDisks2 sends us null terminated strings, make sure to strip the extranous \0 in favor of the implicit \0.
// Otherwise comparision becomes unnecessarily complicated because 'foo\0' != 'foo'. QByteArrays are implicitly
// terminated already.
void DeviceBackend::cacheProperty(const QString &key, const QVariant &value) const
{
    if (value.metaType() == QMetaType::fromType<QByteArray>()) {
        auto blob = value.toByteArray();
        while (blob.endsWith('\0')) {
            blob.chop(1);
        }
        m_propertyCache.insert(key, blob);
    } else {
        m_propertyCache.insert(key, value);
    }
}

#include "moc_udisksdevicebackend.cpp"
