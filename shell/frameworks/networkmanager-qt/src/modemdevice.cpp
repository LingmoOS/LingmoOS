/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2011-2013 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modemdevice.h"
#include "modemdevice_p.h"

#include "manager_p.h"

NetworkManager::ModemDevice::Capabilities convertModemCapabilities(uint theirCaps)
{
    NetworkManager::ModemDevice::Capabilities ourCaps = (NetworkManager::ModemDevice::Capabilities)theirCaps;
    return ourCaps;
}

NetworkManager::ModemDevicePrivate::ModemDevicePrivate(const QString &path, ModemDevice *q)
    : DevicePrivate(path, q)
#ifdef NMQT_STATIC
    , modemIface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    , modemIface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
{
}

NetworkManager::ModemDevice::ModemDevice(const QString &path, QObject *parent)
    : Device(*new ModemDevicePrivate(path, this), parent)
{
    Q_D(ModemDevice);

    QVariantMap initialProperties = NetworkManagerPrivate::retrieveInitialProperties(d->modemIface.staticInterfaceName(), path);
    if (!initialProperties.isEmpty()) {
        d->propertiesChanged(initialProperties);
    }
}

NetworkManager::ModemDevice::ModemDevice(NetworkManager::ModemDevicePrivate &dd, QObject *parent)
    : Device(dd, parent)
{
    Q_D(ModemDevice);

    QDBusConnection::systemBus().connect(NetworkManagerPrivate::DBUS_SERVICE,
                                         d->uni,
                                         NetworkManagerPrivate::FDO_DBUS_PROPERTIES,
                                         QLatin1String("PropertiesChanged"),
                                         d,
                                         SLOT(dbusPropertiesChanged(QString, QVariantMap, QStringList)));
}

NetworkManager::ModemDevice::~ModemDevice()
{
}

NetworkManager::Device::Type NetworkManager::ModemDevice::type() const
{
    return NetworkManager::Device::Modem;
}

NetworkManager::ModemDevice::Capabilities NetworkManager::ModemDevice::currentCapabilities() const
{
    Q_D(const ModemDevice);
    return d->currentCapabilities;
}

NetworkManager::ModemDevice::Capabilities NetworkManager::ModemDevice::modemCapabilities() const
{
    Q_D(const ModemDevice);
    return d->modemCapabilities;
}

void NetworkManager::ModemDevicePrivate::propertyChanged(const QString &property, const QVariant &value)
{
    Q_Q(ModemDevice);

    if (property == QLatin1String("CurrentCapabilities")) {
        currentCapabilities = convertModemCapabilities(value.toUInt());
        Q_EMIT q->currentCapabilitiesChanged(currentCapabilities);
    } else if (property == QLatin1String("ModemCapabilities")) {
        modemCapabilities = convertModemCapabilities(value.toUInt());
    } else {
        DevicePrivate::propertyChanged(property, value);
    }
}

#include "moc_modemdevice.cpp"
#include "moc_modemdevice_p.cpp"
