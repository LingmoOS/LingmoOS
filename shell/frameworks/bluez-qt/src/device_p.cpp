/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "device_p.h"
#include "device.h"
#include "gattserviceremote_p.h"
#include "gattserviceremote.h"
#include "adapter.h"
#include "battery.h"
#include "battery_p.h"
#include "device.h"
#include "input.h"
#include "input_p.h"
#include "macros.h"
#include "mediaplayer.h"
#include "mediaplayer_p.h"
#include "mediatransport.h"
#include "mediatransport_p.h"
#include "utils.h"

namespace BluezQt
{
static const qint16 INVALID_RSSI = -32768; // qint16 minimum

DevicePrivate::DevicePrivate(const QString &path, const QVariantMap &properties, const AdapterPtr &adapter)
    : QObject()
    , m_dbusProperties(nullptr)
    , m_deviceClass(0)
    , m_appearance(0)
    , m_paired(false)
    , m_trusted(false)
    , m_blocked(false)
    , m_legacyPairing(false)
    , m_rssi(INVALID_RSSI)
    , m_manufacturerData(ManData())
    , m_servicesResolved(false)
    , m_connected(false)
    , m_adapter(adapter)
{
    m_bluezDevice = new BluezDevice(Strings::orgBluez(), path, DBusConnection::orgBluez(), this);

    init(properties);
}

static QHash<QString, QByteArray> toByteArrayHash(const QDBusArgument &arg)
{
    if (arg.currentType() != QDBusArgument::MapType) {
        return {};
    }

    QHash<QString, QByteArray> result;
    arg.beginMap();
    while (!arg.atEnd()) {
        arg.beginMapEntry();
        QString key;
        QDBusVariant value;
        arg >> key >> value;
        result.insert(key.toUpper(), value.variant().toByteArray());
        arg.endMapEntry();
    }
    arg.endMap();
    return result;
}

void DevicePrivate::init(const QVariantMap &properties)
{
    m_dbusProperties = new DBusProperties(Strings::orgBluez(), m_bluezDevice->path(), DBusConnection::orgBluez(), this);

    // Init properties
    m_address = properties.value(QStringLiteral("Address")).toString();
    m_name = properties.value(QStringLiteral("Name")).toString();
    m_alias = properties.value(QStringLiteral("Alias")).toString();
    m_deviceClass = properties.value(QStringLiteral("Class")).toUInt();
    m_appearance = properties.value(QStringLiteral("Appearance")).toUInt();
    m_icon = properties.value(QStringLiteral("Icon")).toString();
    m_paired = properties.value(QStringLiteral("Paired")).toBool();
    m_trusted = properties.value(QStringLiteral("Trusted")).toBool();
    m_blocked = properties.value(QStringLiteral("Blocked")).toBool();
    m_legacyPairing = properties.value(QStringLiteral("LegacyPairing")).toBool();
    m_rssi = properties.value(QStringLiteral("RSSI")).toInt();
    m_manufacturerData = variantToManData(properties.value(QStringLiteral("ManufacturerData")));
    m_servicesResolved = properties.value(QStringLiteral("ServicesResolved")).toBool();
    m_connected = properties.value(QStringLiteral("Connected")).toBool();
    m_uuids = stringListToUpper(properties.value(QStringLiteral("UUIDs")).toStringList());
    m_modalias = properties.value(QStringLiteral("Modalias")).toString();
    m_serviceData = toByteArrayHash(properties.value(QStringLiteral("ServiceData")).value<QDBusArgument>());

    if (!m_rssi) {
        m_rssi = INVALID_RSSI;
    }
}

void DevicePrivate::interfacesAdded(const QString &path, const QVariantMapMap &interfaces)
{
    bool changed = false;
    QVariantMapMap::const_iterator it;

    for (it = interfaces.constBegin(); it != interfaces.constEnd(); ++it) {
        if (it.key() == Strings::orgBluezBattery1()) {
            m_battery = BatteryPtr(new Battery(path, it.value()));
            m_battery->d->q = m_battery.toWeakRef();
            Q_EMIT q.lock()->batteryChanged(m_battery);
            changed = true;
        } else if (it.key() == Strings::orgBluezInput1()) {
            m_input = InputPtr(new Input(path, it.value()));
            m_input->d->q = m_input.toWeakRef();
            Q_EMIT q.lock()->inputChanged(m_input);
            changed = true;
        } else if (it.key() == Strings::orgBluezMediaPlayer1()) {
            m_mediaPlayer = MediaPlayerPtr(new MediaPlayer(path, it.value()));
            m_mediaPlayer->d->q = m_mediaPlayer.toWeakRef();
            Q_EMIT q.lock()->mediaPlayerChanged(m_mediaPlayer);
            changed = true;
        } else if (it.key() == Strings::orgBluezMediaTransport1()) {
            m_mediaTransport = MediaTransportPtr(new MediaTransport(path, it.value()));
            m_mediaTransport->d->q = m_mediaTransport.toWeakRef();
            Q_EMIT q.lock()->mediaTransportChanged(m_mediaTransport);
            changed = true;
        } else if (it.key() == Strings::orgBluezGattService1()) {
            addGattService(path,it.value());
            changed = true;
        }
    }

    for (auto& service : m_services) {
        if (path.startsWith(service->ubi())) {
            service->d->interfacesAdded(path, interfaces);
            changed = true;
        }
    }

    if (changed) {
        Q_EMIT q.lock()->deviceChanged(q.toStrongRef());
    }
}

void DevicePrivate::interfacesRemoved(const QString &path, const QStringList &interfaces)
{
    bool changed = false;

    for (const QString &interface : interfaces) {
        if (interface == Strings::orgBluezBattery1() && m_battery && m_battery->d->m_path == path) {
            m_battery.clear();
            Q_EMIT q.lock()->batteryChanged(m_battery);
            changed = true;
        } else if (interface == Strings::orgBluezInput1() && m_input && m_input->d->m_path == path) {
            m_input.clear();
            Q_EMIT q.lock()->inputChanged(m_input);
            changed = true;
        } else if (interface == Strings::orgBluezMediaPlayer1() && m_mediaPlayer && m_mediaPlayer->d->m_path == path) {
            m_mediaPlayer.clear();
            Q_EMIT q.lock()->mediaPlayerChanged(m_mediaPlayer);
            changed = true;
        } else if (interface == Strings::orgBluezMediaTransport1() && m_mediaTransport && m_mediaTransport->d->m_path == path) {
            m_mediaTransport.clear();
            Q_EMIT q.lock()->mediaTransportChanged(m_mediaTransport);
            changed = true;
        } else if (interface == Strings::orgBluezGattService1()) {
            removeGattService(path);
            changed = true;
        }
    }

    for (auto& service : m_services) {
        if (path.startsWith(service->ubi())) {
            service->d->interfacesRemoved(path,interfaces);
            changed = true;
        }
    }

    if (changed) {
        Q_EMIT q.lock()->deviceChanged(q.toStrongRef());
    }
}

void DevicePrivate::addGattService(const QString &gattServicePath, const QVariantMap &properties)
{
    // Check if we have the right path
    if (m_bluezDevice->path() != properties.value(QStringLiteral("Device")).value<QDBusObjectPath>().path()) {
        return;
    }

    DevicePtr device = DevicePtr(this->q);

    if (!device) {
        return;
    }

    GattServiceRemotePtr gattService = GattServiceRemotePtr(new GattServiceRemote(gattServicePath, properties, device));
    gattService->d->q = gattService.toWeakRef();
    m_services.append(gattService);

    Q_EMIT device->gattServiceAdded(gattService);
    Q_EMIT device->gattServicesChanged(m_services);

    // Connections
    connect(gattService.data(),&GattServiceRemote::serviceChanged,q.lock().data(),&Device::gattServiceChanged);
}

void DevicePrivate::removeGattService(const QString &gattServicePath)
{
    DevicePtr device = DevicePtr(this->q);

    if (!device) {
        return;
    }

    GattServiceRemotePtr gattService = nullptr;
    for (int i=0; i < device->gattServices().size(); ++i) {
        if (device->gattServices().at(i)->ubi() == gattServicePath) {
            gattService = device->gattServices().at(i);
        }
    }

    if (gattService == nullptr) {
        return;
    }

    m_services.removeOne(gattService);

    Q_EMIT device->gattServiceRemoved(gattService);
    Q_EMIT device->gattServicesChanged(m_services);

    // Connections
    disconnect(gattService.data(),&GattServiceRemote::serviceChanged,q.lock().data(),&Device::gattServiceChanged);
}

QDBusPendingReply<> DevicePrivate::setDBusProperty(const QString &name, const QVariant &value)
{
    return m_dbusProperties->Set(Strings::orgBluezDevice1(), name, QDBusVariant(value));
}

void DevicePrivate::propertiesChanged(const QString &path, const QString &interface, const QVariantMap &changed, const QStringList &invalidated)
{
    if (interface == Strings::orgBluezBattery1() && m_battery) {
        m_battery->d->propertiesChanged(interface, changed, invalidated);
    } else if (interface == Strings::orgBluezInput1() && m_input) {
        m_input->d->propertiesChanged(interface, changed, invalidated);
    } else if (interface == Strings::orgBluezMediaPlayer1() && m_mediaPlayer) {
        m_mediaPlayer->d->propertiesChanged(interface, changed, invalidated);
    } else if ((interface == Strings::orgBluezGattService1()) || (interface == Strings::orgBluezGattCharacteristic1()) || (interface == Strings::orgBluezGattDescriptor1())) {
        for (GattServiceRemotePtr service : m_services) {
            if (path.startsWith(service->ubi())) {
                service->d->propertiesChanged(path, interface, changed, invalidated);
                return;
            }
        }
    } else if (interface != Strings::orgBluezDevice1()) {
        return;
    }

    QVariantMap::const_iterator i;
    for (i = changed.constBegin(); i != changed.constEnd(); ++i) {
        const QVariant &value = i.value();
        const QString &property = i.key();

        if (property == QLatin1String("Name")) {
            namePropertyChanged(value.toString());
        } else if (property == QLatin1String("Address")) {
            addressPropertyChanged(value.toString());
        } else if (property == QLatin1String("Alias")) {
            aliasPropertyChanged(value.toString());
        } else if (property == QLatin1String("Class")) {
            classPropertyChanged(value.toUInt());
        } else if (property == QLatin1String("Appearance")) {
            PROPERTY_CHANGED(m_appearance, toUInt, appearanceChanged);
        } else if (property == QLatin1String("Icon")) {
            PROPERTY_CHANGED(m_icon, toString, iconChanged);
        } else if (property == QLatin1String("Paired")) {
            PROPERTY_CHANGED(m_paired, toBool, pairedChanged);
        } else if (property == QLatin1String("Trusted")) {
            PROPERTY_CHANGED(m_trusted, toBool, trustedChanged);
        } else if (property == QLatin1String("Blocked")) {
            PROPERTY_CHANGED(m_blocked, toBool, blockedChanged);
        } else if (property == QLatin1String("LegacyPairing")) {
            PROPERTY_CHANGED(m_legacyPairing, toBool, legacyPairingChanged);
        } else if (property == QLatin1String("RSSI")) {
            PROPERTY_CHANGED(m_rssi, toInt, rssiChanged);
        } else if (property == QLatin1String("ManufacturerData")) {
            PROPERTY_CHANGED2(m_manufacturerData, variantToManData(value), manufacturerDataChanged);
        } else if (property == QLatin1String("ServicesResolved")) {
            PROPERTY_CHANGED(m_servicesResolved, toBool, servicesResolvedChanged);
        } else if (property == QLatin1String("Connected")) {
            PROPERTY_CHANGED(m_connected, toBool, connectedChanged);
        } else if (property == QLatin1String("Modalias")) {
            PROPERTY_CHANGED(m_modalias, toString, modaliasChanged);
        } else if (property == QLatin1String("UUIDs")) {
            PROPERTY_CHANGED2(m_uuids, stringListToUpper(value.toStringList()), uuidsChanged);
        } else if (property == QLatin1String("ServiceData")) {
            PROPERTY_CHANGED2(m_serviceData, toByteArrayHash(value.value<QDBusArgument>()), serviceDataChanged);
        }
    }

    for (const QString &property : invalidated) {
        if (property == QLatin1String("Name")) {
            namePropertyChanged(QString());
        } else if (property == QLatin1String("Class")) {
            classPropertyChanged(0);
        } else if (property == QLatin1String("Appearance")) {
            PROPERTY_INVALIDATED(m_appearance, 0, appearanceChanged);
        } else if (property == QLatin1String("Icon")) {
            PROPERTY_INVALIDATED(m_icon, QString(), iconChanged);
        } else if (property == QLatin1String("RSSI")) {
            PROPERTY_INVALIDATED(m_rssi, INVALID_RSSI, rssiChanged);
        } else if (property == QLatin1String("ManufacturerData")) {
            QMap<uint16_t,QByteArray> map;
            PROPERTY_INVALIDATED(m_manufacturerData, map, manufacturerDataChanged);
        } else if (property == QLatin1String("ServicesResolved")) {
            PROPERTY_INVALIDATED(m_servicesResolved, false, servicesResolvedChanged);
        } else if (property == QLatin1String("Modalias")) {
            PROPERTY_INVALIDATED(m_modalias, QString(), modaliasChanged);
        } else if (property == QLatin1String("UUIDs")) {
            PROPERTY_INVALIDATED(m_uuids, QStringList(), uuidsChanged);
        } else if (property == QLatin1String("ServiceData")) {
            PROPERTY_INVALIDATED(m_serviceData, (QHash<QString, QByteArray>()), serviceDataChanged);
        }
    }

    Q_EMIT q.lock()->deviceChanged(q.toStrongRef());
}

void DevicePrivate::namePropertyChanged(const QString &value)
{
    if (m_name != value) {
        m_name = value;
        Q_EMIT q.lock()->remoteNameChanged(m_name);
        Q_EMIT q.lock()->friendlyNameChanged(q.lock()->friendlyName());
    }
}

void DevicePrivate::addressPropertyChanged(const QString &value)
{
    if (m_address != value) {
        m_address = value;
        Q_EMIT q.lock()->addressChanged(m_address);
    }
}

void DevicePrivate::aliasPropertyChanged(const QString &value)
{
    if (m_alias != value) {
        m_alias = value;
        Q_EMIT q.lock()->nameChanged(m_alias);
        Q_EMIT q.lock()->friendlyNameChanged(q.lock()->friendlyName());
    }
}

void DevicePrivate::classPropertyChanged(quint32 value)
{
    if (m_deviceClass != value) {
        m_deviceClass = value;
        Q_EMIT q.lock()->deviceClassChanged(m_deviceClass);
        Q_EMIT q.lock()->typeChanged(q.lock()->type());
    }
}

void DevicePrivate::serviceDataChanged(const QHash<QString, QByteArray> &value)
{
    if (m_serviceData != value) {
        m_serviceData = value;
        Q_EMIT q.lock()->serviceDataChanged(m_serviceData);
    }
}

} // namespace BluezQt

#include "moc_device_p.cpp"
