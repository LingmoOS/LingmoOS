/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "wirelessdevice.h"
#include "wirelessdevice_p.h"

#undef signals
#include <libnm/NetworkManager.h>
#define signals Q_SIGNALS

#include "manager_p.h"

#include "nmdebug.h"
#include "utils.h"

#include <QDBusMetaType>

NetworkManager::WirelessDevicePrivate::WirelessDevicePrivate(const QString &path, WirelessDevice *q)
    : DevicePrivate(path, q)
#ifdef NMQT_STATIC
    , wirelessIface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    , wirelessIface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
    , bitRate(0)
{
}

NetworkManager::WirelessDevice::WirelessDevice(const QString &path, QObject *parent)
    : Device(*new WirelessDevicePrivate(path, this), parent)
{
    Q_D(WirelessDevice);

    qDBusRegisterMetaType<QList<QDBusObjectPath>>();

#ifdef NMQT_STATIC
    connect(&d->wirelessIface, &OrgFreedesktopNetworkManagerDeviceWirelessInterface::PropertiesChanged, d, &WirelessDevicePrivate::propertiesChanged);
#endif

    connect(&d->wirelessIface, &OrgFreedesktopNetworkManagerDeviceWirelessInterface::AccessPointAdded, d, &WirelessDevicePrivate::accessPointAdded);
    connect(&d->wirelessIface, &OrgFreedesktopNetworkManagerDeviceWirelessInterface::AccessPointRemoved, d, &WirelessDevicePrivate::accessPointRemoved);

    const QList<QDBusObjectPath> aps = d->wirelessIface.accessPoints();
    // qCDebug(NMQT) << "AccessPoint list";
    for (const QDBusObjectPath &op : aps) {
        // qCDebug(NMQT) << "  " << op.path();
        d->accessPointAdded(op);
    }

    // Get all WirelessDevices's properties at once
    QVariantMap initialProperties = NetworkManagerPrivate::retrieveInitialProperties(d->wirelessIface.staticInterfaceName(), path);
    if (!initialProperties.isEmpty()) {
        d->propertiesChanged(initialProperties);
    }

}

NetworkManager::WirelessDevice::~WirelessDevice()
{
}

NetworkManager::Device::Type NetworkManager::WirelessDevice::type() const
{
    return NetworkManager::Device::Wifi;
}

QStringList NetworkManager::WirelessDevice::accessPoints() const
{
    Q_D(const WirelessDevice);
    return d->apMap.keys();
}

QDBusPendingReply<> NetworkManager::WirelessDevice::requestScan(const QVariantMap &options)
{
    Q_D(WirelessDevice);
    d->lastRequestScan = QDateTime::currentDateTime();
    return d->wirelessIface.RequestScan(options);
}

NetworkManager::AccessPoint::Ptr NetworkManager::WirelessDevice::activeAccessPoint() const
{
    Q_D(const WirelessDevice);
    return d->activeAccessPoint;
}

QString NetworkManager::WirelessDevice::hardwareAddress() const
{
    Q_D(const WirelessDevice);
    return d->hardwareAddress;
}

QString NetworkManager::WirelessDevice::permanentHardwareAddress() const
{
    Q_D(const WirelessDevice);
    return d->permanentHardwareAddress;
}

NetworkManager::WirelessDevice::OperationMode NetworkManager::WirelessDevice::mode() const
{
    Q_D(const WirelessDevice);
    return d->mode;
}

int NetworkManager::WirelessDevice::bitRate() const
{
    Q_D(const WirelessDevice);
    return d->bitRate;
}

QDateTime NetworkManager::WirelessDevice::lastScan() const
{
    Q_D(const WirelessDevice);
    return d->lastScan;
}

QDateTime NetworkManager::WirelessDevice::lastRequestScan() const
{
    Q_D(const WirelessDevice);
    return d->lastRequestScan;
}

NetworkManager::WirelessDevice::Capabilities NetworkManager::WirelessDevice::wirelessCapabilities() const
{
    Q_D(const WirelessDevice);
    return d->wirelessCapabilities;
}

NetworkManager::AccessPoint::Ptr NetworkManager::WirelessDevice::findAccessPoint(const QString &uni)
{
    Q_D(WirelessDevice);
    NetworkManager::AccessPoint::Ptr accessPoint;

    QMap<QString, NetworkManager::AccessPoint::Ptr>::ConstIterator mapIt = d->apMap.constFind(uni);
    if (mapIt != d->apMap.constEnd()) {
        accessPoint = mapIt.value();
    } else if (!uni.isEmpty() && uni != QLatin1String("/")) {
        d->accessPointAdded(QDBusObjectPath(uni));
        mapIt = d->apMap.constFind(uni);
        if (mapIt != d->apMap.constEnd()) {
            accessPoint = mapIt.value();
        }
    }

    return accessPoint;
}

NetworkManager::WirelessNetwork::List NetworkManager::WirelessDevice::networks() const
{
    Q_D(const WirelessDevice);
    return d->networks.values();
}

NetworkManager::WirelessNetwork::Ptr NetworkManager::WirelessDevice::findNetwork(const QString &ssid) const
{
    Q_D(const WirelessDevice);
    NetworkManager::WirelessNetwork::Ptr ret;
    if (d->networks.contains(ssid)) {
        ret = d->networks.value(ssid);
    }
    return ret;
}

void NetworkManager::WirelessDevicePrivate::accessPointAdded(const QDBusObjectPath &accessPoint)
{
    // kDebug(1441) << apPath.path();
    Q_Q(WirelessDevice);

    if (!apMap.contains(accessPoint.path())) {
        NetworkManager::AccessPoint::Ptr accessPointPtr(new NetworkManager::AccessPoint(accessPoint.path()), &QObject::deleteLater);
        apMap.insert(accessPoint.path(), accessPointPtr);
        Q_EMIT q->accessPointAppeared(accessPoint.path());

        const QString ssid = accessPointPtr->ssid();
        if (!ssid.isEmpty() && !networks.contains(ssid)) {
            NetworkManager::WirelessNetwork::Ptr wifiNetwork(new NetworkManager::WirelessNetwork(accessPointPtr, q), &QObject::deleteLater);
            networks.insert(ssid, wifiNetwork);
            connect(wifiNetwork.data(), &WirelessNetwork::disappeared, this, &WirelessDevicePrivate::removeNetwork);
            Q_EMIT q->networkAppeared(ssid);
        }
    }
}

void NetworkManager::WirelessDevicePrivate::accessPointRemoved(const QDBusObjectPath &accessPoint)
{
    // kDebug(1441) << apPath.path();
    Q_Q(WirelessDevice);
    if (!apMap.contains(accessPoint.path())) {
        qCDebug(NMQT) << "Access point list lookup failed for " << accessPoint.path();
    }
    Q_EMIT q->accessPointDisappeared(accessPoint.path());
    apMap.remove(accessPoint.path());
}

void NetworkManager::WirelessDevicePrivate::removeNetwork(const QString &network)
{
    Q_Q(WirelessDevice);

    if (networks.contains(network)) {
        networks.remove(network);
        Q_EMIT q->networkDisappeared(network);
    }
}

void NetworkManager::WirelessDevicePrivate::propertyChanged(const QString &property, const QVariant &value)
{
    Q_Q(WirelessDevice);

    if (property == QLatin1String("ActiveAccessPoint")) {
        QDBusObjectPath activeAccessPointTmp = qdbus_cast<QDBusObjectPath>(value);
        activeAccessPoint = q->findAccessPoint(activeAccessPointTmp.path());
        Q_EMIT q->activeAccessPointChanged(activeAccessPointTmp.path());
    } else if (property == QLatin1String("HwAddress")) {
        hardwareAddress = value.toString();
        Q_EMIT q->hardwareAddressChanged(hardwareAddress);
    } else if (property == QLatin1String("PermHwAddress")) {
        permanentHardwareAddress = value.toString();
        Q_EMIT q->permanentHardwareAddressChanged(permanentHardwareAddress);
    } else if (property == QLatin1String("Bitrate")) {
        bitRate = value.toUInt();
        Q_EMIT q->bitRateChanged(bitRate);
    } else if (property == QLatin1String("Mode")) {
        mode = q->convertOperationMode(value.toUInt());
        Q_EMIT q->modeChanged(mode);
    } else if (property == QLatin1String("WirelessCapabilities")) {
        wirelessCapabilities = q->convertCapabilities(value.toUInt());
        Q_EMIT q->wirelessCapabilitiesChanged(wirelessCapabilities);
    } else if (property == QLatin1String("LastScan")) {
        lastScan = NetworkManager::clockBootTimeToDateTime(value.toLongLong());
        Q_EMIT q->lastScanChanged(lastScan);
    } else if (property == QLatin1String("AccessPoints")) {
        // TODO use this instead AccessPointAdded/Removed signals?
    } else {
        DevicePrivate::propertyChanged(property, value);
    }
}

NetworkManager::WirelessDevice::OperationMode NetworkManager::WirelessDevice::convertOperationMode(uint theirMode)
{
    NetworkManager::WirelessDevice::OperationMode ourMode = NetworkManager::WirelessDevice::Unknown;
    switch (theirMode) {
    case NM_802_11_MODE_UNKNOWN:
        ourMode = NetworkManager::WirelessDevice::Unknown;
        break;
    case NM_802_11_MODE_ADHOC:
        ourMode = NetworkManager::WirelessDevice::Adhoc;
        break;
    case NM_802_11_MODE_INFRA:
        ourMode = NetworkManager::WirelessDevice::Infra;
        break;
    case NM_802_11_MODE_AP:
        ourMode = NetworkManager::WirelessDevice::ApMode;
        break;
    default:
        qCDebug(NMQT) << Q_FUNC_INFO << "Unhandled mode" << theirMode;
    }
    return ourMode;
}

NetworkManager::WirelessDevice::Capabilities NetworkManager::WirelessDevice::convertCapabilities(uint caps)
{
    return (NetworkManager::WirelessDevice::Capabilities)caps;
}

#include "moc_wirelessdevice.cpp"
#include "moc_wirelessdevice_p.cpp"
