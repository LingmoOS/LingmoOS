/*
    SPDX-FileCopyrightText: 2010 Michael Zanetti <mzanetti@kde.org>
    SPDX-FileCopyrightText: 2010 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "upowerdevice.h"
#include "upower.h"
#include "upowerbattery.h"
#include "upowerdeviceinterface.h"
#include "upowergenericinterface.h"

#include <solid/device.h>
#include <solid/genericinterface.h>

#include <QDBusConnection>
#include <QDBusReply>
#include <QStringList>

using namespace Solid::Backends::UPower;

UPowerDevice::UPowerDevice(const QString &udi)
    : Solid::Ifaces::Device()
    , m_udi(udi)
{
    QDBusConnection::systemBus().connect(UP_DBUS_SERVICE,
                                         m_udi,
                                         "org.freedesktop.DBus.Properties",
                                         "PropertiesChanged",
                                         this,
                                         SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));

    // TODO port this to Solid::Power, we can't link against kdelibs4support for this signal
    // older upower versions not affected
    QDBusConnection::systemBus().connect("org.freedesktop.login1", //
                                         "/org/freedesktop/login1",
                                         "org.freedesktop.login1.Manager",
                                         "PrepareForSleep",
                                         this,
                                         SLOT(login1Resuming(bool)));
}

UPowerDevice::~UPowerDevice()
{
}

QObject *UPowerDevice::createDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    if (!queryDeviceInterface(type)) {
        return nullptr;
    }

    DeviceInterface *iface = nullptr;
    switch (type) {
    case Solid::DeviceInterface::GenericInterface:
        iface = new GenericInterface(this);
        break;
    case Solid::DeviceInterface::Battery:
        iface = new Battery(this);
        break;
    default:
        break;
    }
    return iface;
}

bool UPowerDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    const UpDeviceKind uptype = static_cast<UpDeviceKind>(prop("Type").toUInt());
    switch (type) {
    case Solid::DeviceInterface::GenericInterface:
        return true;
    case Solid::DeviceInterface::Battery:
        switch (uptype) {
        case UP_DEVICE_KIND_BATTERY:
        case UP_DEVICE_KIND_UPS:
        case UP_DEVICE_KIND_MOUSE:
        case UP_DEVICE_KIND_KEYBOARD:
        case UP_DEVICE_KIND_PDA:
        case UP_DEVICE_KIND_PHONE:
        case UP_DEVICE_KIND_TABLET:
        case UP_DEVICE_KIND_GAMING_INPUT:
        case UP_DEVICE_KIND_SPEAKERS:
        case UP_DEVICE_KIND_HEADSET:
        case UP_DEVICE_KIND_HEADPHONES:
        case UP_DEVICE_KIND_BLUETOOTH_GENERIC:
        case UP_DEVICE_KIND_TOUCHPAD:
            return true;
        case UP_DEVICE_KIND_UNKNOWN:
            // There is currently no "Bluetooth battery" type, so check if it comes from Bluez
            if (prop("NativePath").toString().startsWith(QLatin1String("/org/bluez/"))) {
                return true;
            }
            return false;
        case UP_DEVICE_KIND_LINE_POWER:
        case UP_DEVICE_KIND_MONITOR:
        case UP_DEVICE_KIND_MEDIA_PLAYER:
        case UP_DEVICE_KIND_COMPUTER:
        case UP_DEVICE_KIND_LAST:
            return false;
        }
        Q_FALLTHROUGH();
    default:
        return false;
    }
}

QStringList UPowerDevice::emblems() const
{
    return QStringList();
}

QString UPowerDevice::description() const
{
    if (queryDeviceInterface(Solid::DeviceInterface::Battery)) {
        return tr("%1 Battery", "%1 is battery technology").arg(batteryTechnology());
    } else {
        QString result = prop("Model").toString();
        if (result.isEmpty()) {
            return vendor();
        }
        return result;
    }
}

QString UPowerDevice::batteryTechnology() const
{
    const UpDeviceTechnology tech = static_cast<UpDeviceTechnology>(prop("Technology").toUInt());
    switch (tech) {
    case UP_DEVICE_TECHNOLOGY_UNKNOWN:
        return tr("Unknown", "battery technology");
    case UP_DEVICE_TECHNOLOGY_LITHIUM_ION:
        return tr("Lithium Ion", "battery technology");
    case UP_DEVICE_TECHNOLOGY_LITHIUM_POLYMER:
        return tr("Lithium Polymer", "battery technology");
    case UP_DEVICE_TECHNOLOGY_LITHIUM_IRON_PHOSPHATE:
        return tr("Lithium Iron Phosphate", "battery technology");
    case UP_DEVICE_TECHNOLOGY_LEAD_ACID:
        return tr("Lead Acid", "battery technology");
    case UP_DEVICE_TECHNOLOGY_NICKEL_CADMIUM:
        return tr("Nickel Cadmium", "battery technology");
    case UP_DEVICE_TECHNOLOGY_NICKEL_METAL_HYDRIDE:
        return tr("Nickel Metal Hydride", "battery technology");
    case UP_DEVICE_TECHNOLOGY_LAST:
        return tr("Unknown", "battery technology");
    }
    return tr("Unknown", "battery technology");
}

QString UPowerDevice::icon() const
{
    if (queryDeviceInterface(Solid::DeviceInterface::Battery)) {
        return "battery";
    } else {
        return QString();
    }
}

QString UPowerDevice::product() const
{
    QString result = prop("Model").toString();

    if (result.isEmpty()) {
        result = description();
    }

    return result;
}

QString UPowerDevice::vendor() const
{
    return prop("Vendor").toString();
}

QString UPowerDevice::udi() const
{
    return m_udi;
}

QString UPowerDevice::parentUdi() const
{
    return UP_UDI_PREFIX;
}

void UPowerDevice::checkCache(const QString &key) const
{
    if (m_cache.contains(key) || m_negativeCache.contains(key)) {
        return;
    }

    loadCache();

    if (m_cache.contains(key)) {
        return;
    }

    QDBusMessage call = QDBusMessage::createMethodCall(UP_DBUS_SERVICE, m_udi, "org.freedesktop.DBus.Properties", "Get");
    call.setArguments({UP_DBUS_INTERFACE_DEVICE, key});
    QDBusReply<QVariant> reply = QDBusConnection::systemBus().call(call);

    if (reply.isValid()) {
        m_cache[key] = reply.value();
    } else {
        m_negativeCache.append(key);
    }
}

QVariant UPowerDevice::prop(const QString &key) const
{
    checkCache(key);
    return m_cache.value(key);
}

bool UPowerDevice::propertyExists(const QString &key) const
{
    checkCache(key);
    return m_cache.contains(key);
}

void UPowerDevice::loadCache() const
{
    QDBusMessage call = QDBusMessage::createMethodCall(UP_DBUS_SERVICE, m_udi, "org.freedesktop.DBus.Properties", "GetAll");
    call.setArguments({UP_DBUS_INTERFACE_DEVICE});
    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(call);

    if (reply.isValid()) {
        m_cache = reply.value();
        m_cacheComplete = true;
    } else {
        m_cache.clear();
    }
}

QMap<QString, QVariant> UPowerDevice::allProperties() const
{
    if (!m_cacheComplete) {
        loadCache();
    }

    return m_cache;
}

void UPowerDevice::onPropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps, const QStringList &invalidatedProps)
{
    if (ifaceName != UP_DBUS_INTERFACE_DEVICE)
	return;

    QMap<QString, int> changeMap;
    for (auto it = changedProps.begin(); it != changedProps.end(); ++it) {
        m_cache[it.key()] = it.value();
        m_negativeCache.removeOne(it.key());
        changeMap.insert(it.key(), Solid::GenericInterface::PropertyModified);
    }
    for (const auto &propName : invalidatedProps) {
        m_cache.remove(propName);
        m_negativeCache.removeOne(propName);
        changeMap.insert(propName, Solid::GenericInterface::PropertyModified);
        m_cacheComplete = false;
    }
    Q_EMIT propertyChanged(changeMap);
}

void UPowerDevice::login1Resuming(bool active)
{
    // Nothing to do when going into sleep
    if (active)
        return;

    QMap<QString, int> changeMap;
    for (auto it = m_cache.begin(); it != m_cache.end(); ++it) {
        if (it.value().isValid()) {
            changeMap.insert(it.key(), Solid::GenericInterface::PropertyModified);
	}
    }
    m_cache.clear();
    m_negativeCache.clear();
    m_cacheComplete = false;
    Q_EMIT propertyChanged(changeMap);
}

#include "moc_upowerdevice.cpp"
