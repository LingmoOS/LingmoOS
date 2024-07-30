/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "wimaxdevice.h"
#include "manager_p.h"
#include "wimaxdevice_p.h"

#include "nmdebug.h"

#include <QDBusMetaType>

NetworkManager::WimaxDevicePrivate::WimaxDevicePrivate(const QString &path, WimaxDevice *q)
    : DevicePrivate(path, q)
#ifdef NMQT_STATIC
    , wimaxIface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    , wimaxIface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
{
    qDBusRegisterMetaType<QList<QDBusObjectPath>>();
    const QList<QDBusObjectPath> nsps = wimaxIface.nsps();
    for (const QDBusObjectPath &op : nsps) {
        nspMap.insert(op.path(), NetworkManager::WimaxNsp::Ptr());
        // qCDebug(NMQT) << "  " << op.path();
    }
}

NetworkManager::WimaxDevice::WimaxDevice(const QString &path, QObject *parent)
    : Device(*new WimaxDevicePrivate(path, this), parent)
{
    Q_D(WimaxDevice);

    connect(&d->wimaxIface, &OrgFreedesktopNetworkManagerDeviceWiMaxInterface::NspAdded, d, &WimaxDevicePrivate::nspAdded);
    connect(&d->wimaxIface, &OrgFreedesktopNetworkManagerDeviceWiMaxInterface::NspRemoved, d, &WimaxDevicePrivate::nspRemoved);

    QVariantMap initialProperties = NetworkManagerPrivate::retrieveInitialProperties(d->wimaxIface.staticInterfaceName(), path);
    if (!initialProperties.isEmpty()) {
        d->propertiesChanged(initialProperties);
    }
}

NetworkManager::WimaxDevice::~WimaxDevice()
{
}

NetworkManager::Device::Type NetworkManager::WimaxDevice::type() const
{
    return NetworkManager::Device::Wimax;
}

QStringList NetworkManager::WimaxDevice::nsps() const
{
    Q_D(const WimaxDevice);
    return d->nspMap.keys();
}

NetworkManager::WimaxNsp::Ptr NetworkManager::WimaxDevice::activeNsp() const
{
    Q_D(const WimaxDevice);
    return findNsp(d->activeNsp);
}

QString NetworkManager::WimaxDevice::hardwareAddress() const
{
    Q_D(const WimaxDevice);
    return d->hardwareAddress;
}

QString NetworkManager::WimaxDevice::bsid() const
{
    Q_D(const WimaxDevice);
    return d->bsid;
}

uint NetworkManager::WimaxDevice::centerFrequency() const
{
    Q_D(const WimaxDevice);
    return d->centerFrequency;
}

int NetworkManager::WimaxDevice::cinr() const
{
    Q_D(const WimaxDevice);
    return d->cinr;
}

int NetworkManager::WimaxDevice::rssi() const
{
    Q_D(const WimaxDevice);
    return d->rssi;
}

int NetworkManager::WimaxDevice::txPower() const
{
    Q_D(const WimaxDevice);
    return d->txPower;
}

NetworkManager::WimaxNsp::Ptr NetworkManager::WimaxDevice::findNsp(const QString &uni) const
{
    Q_D(const WimaxDevice);
    NetworkManager::WimaxNsp::Ptr nsp;
    QMap<QString, NetworkManager::WimaxNsp::Ptr>::ConstIterator mapIt = d->nspMap.constFind(uni);
    if (mapIt != d->nspMap.constEnd() && !mapIt.value().isNull()) {
        nsp = mapIt.value();
    } else {
        nsp = NetworkManager::WimaxNsp::Ptr(new NetworkManager::WimaxNsp(uni), &QObject::deleteLater);
        d->nspMap.insert(uni, nsp);
    }

    return nsp;
}

void NetworkManager::WimaxDevicePrivate::nspAdded(const QDBusObjectPath &nspPath)
{
    // qCDebug(NMQT) << nspPath.path();
    Q_Q(WimaxDevice);
    if (!nspMap.contains(nspPath.path())) {
        nspMap.insert(nspPath.path(), NetworkManager::WimaxNsp::Ptr());
        Q_EMIT q->nspAppeared(nspPath.path());
    }
}

void NetworkManager::WimaxDevicePrivate::nspRemoved(const QDBusObjectPath &nspPath)
{
    // qCDebug(NMQT) << nspPath.path();
    Q_Q(WimaxDevice);
    if (!nspMap.contains(nspPath.path())) {
        qCDebug(NMQT) << "Access point list lookup failed for " << nspPath.path();
    }
    Q_EMIT q->nspDisappeared(nspPath.path());
    nspMap.remove(nspPath.path());
}

void NetworkManager::WimaxDevicePrivate::propertyChanged(const QString &property, const QVariant &value)
{
    Q_Q(WimaxDevice);

    if (property == QLatin1String("ActiveNsp")) {
        activeNsp = qdbus_cast<QDBusObjectPath>(value).path();
        Q_EMIT q->activeNspChanged(activeNsp);
    } else if (property == QLatin1String("HwAddress")) {
        hardwareAddress = value.toString();
        Q_EMIT q->hardwareAddressChanged(hardwareAddress);
    } else if (property == QLatin1String("Bsid")) {
        bsid = value.toString();
        Q_EMIT q->bsidChanged(bsid);
    } else if (property == QLatin1String("CenterFrequency")) {
        centerFrequency = value.toUInt();
        Q_EMIT q->centerFrequencyChanged(centerFrequency);
    } else if (property == QLatin1String("Cinr")) {
        cinr = value.toInt();
        Q_EMIT q->cinrChanged(cinr);
    } else if (property == QLatin1String("Rssi")) {
        rssi = value.toInt();
        Q_EMIT q->rssiChanged(rssi);
    } else if (property == QLatin1String("TxPower")) {
        txPower = value.toInt();
        Q_EMIT q->txPowerChanged(txPower);
    } else {
        DevicePrivate::propertyChanged(property, value);
    }
}

#include "moc_wimaxdevice.cpp"
#include "moc_wimaxdevice_p.cpp"
