/*
    SPDX-FileCopyrightText: 2008 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "accesspoint_p.h"
#include "manager_p.h"
#include "wirelessdevice.h"

#include "nmdebug.h"

NetworkManager::AccessPointPrivate::AccessPointPrivate(const QString &path, AccessPoint *q)
#ifdef NMQT_STATIC
    : iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    : iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
    , capabilities(AccessPoint::None)
    , wpaFlags()
    , rsnFlags()
    , frequency(0)
    , maxBitRate(0)
    , mode(AccessPoint::Unknown)
    , signalStrength(0)
    , lastSeen(-1)
    , q_ptr(q)
{
    uni = path;
}

NetworkManager::AccessPoint::Capabilities NetworkManager::AccessPointPrivate::convertCapabilities(int caps)
{
    NetworkManager::AccessPoint::Capabilities capFlags;
    if (caps & NM_802_11_AP_FLAGS_PRIVACY) {
        capFlags |= AccessPoint::Privacy;
    }

    return capFlags;
}

NetworkManager::AccessPoint::WpaFlags NetworkManager::AccessPointPrivate::convertWpaFlags(uint theirFlags)
{
    return (AccessPoint::WpaFlags)theirFlags;
}

NetworkManager::AccessPoint::AccessPoint(const QString &path, QObject *parent)
    : QObject(parent)
    , d_ptr(new AccessPointPrivate(path, this))
{
    Q_D(AccessPoint);

    QDBusConnection::systemBus().connect(NetworkManagerPrivate::DBUS_SERVICE,
                                         d->uni,
                                         NetworkManagerPrivate::FDO_DBUS_PROPERTIES,
                                         QLatin1String("PropertiesChanged"),
                                         d,
                                         SLOT(dbusPropertiesChanged(QString, QVariantMap, QStringList)));

    // Get all AccessPoint's properties at once
    QVariantMap initialProperties = NetworkManagerPrivate::retrieveInitialProperties(d->iface.staticInterfaceName(), path);
    if (!initialProperties.isEmpty()) {
        d->propertiesChanged(initialProperties);
    }
}

NetworkManager::AccessPoint::~AccessPoint()
{
    Q_D(AccessPoint);
    delete d;
}

QString NetworkManager::AccessPoint::uni() const
{
    Q_D(const AccessPoint);
    return d->uni;
}

QString NetworkManager::AccessPoint::hardwareAddress() const
{
    Q_D(const AccessPoint);
    return d->hardwareAddress;
}

NetworkManager::AccessPoint::Capabilities NetworkManager::AccessPoint::capabilities() const
{
    Q_D(const AccessPoint);
    return d->capabilities;
}

NetworkManager::AccessPoint::WpaFlags NetworkManager::AccessPoint::wpaFlags() const
{
    Q_D(const AccessPoint);
    return d->wpaFlags;
}

NetworkManager::AccessPoint::WpaFlags NetworkManager::AccessPoint::rsnFlags() const
{
    Q_D(const AccessPoint);
    return d->rsnFlags;
}

QString NetworkManager::AccessPoint::ssid() const
{
    Q_D(const AccessPoint);
    return d->ssid;
}

QByteArray NetworkManager::AccessPoint::rawSsid() const
{
    Q_D(const AccessPoint);
    return d->rawSsid;
}

uint NetworkManager::AccessPoint::frequency() const
{
    Q_D(const AccessPoint);
    return d->frequency;
}

uint NetworkManager::AccessPoint::maxBitRate() const
{
    Q_D(const AccessPoint);
    return d->maxBitRate;
}

NetworkManager::AccessPoint::OperationMode NetworkManager::AccessPoint::mode() const
{
    Q_D(const AccessPoint);
    return d->mode;
}

int NetworkManager::AccessPoint::signalStrength() const
{
    Q_D(const AccessPoint);
    return d->signalStrength;
}

int NetworkManager::AccessPoint::lastSeen() const
{
    Q_D(const AccessPoint);
    return d->lastSeen;
}

NetworkManager::AccessPoint::OperationMode NetworkManager::AccessPoint::convertOperationMode(uint mode)
{
    NetworkManager::AccessPoint::OperationMode ourMode = NetworkManager::AccessPoint::Unknown;
    switch (mode) {
    case NM_802_11_MODE_UNKNOWN:
        ourMode = NetworkManager::AccessPoint::Unknown;
        break;
    case NM_802_11_MODE_ADHOC:
        ourMode = NetworkManager::AccessPoint::Adhoc;
        break;
    case NM_802_11_MODE_INFRA:
        ourMode = NetworkManager::AccessPoint::Infra;
        break;
    case NM_802_11_MODE_AP:
        ourMode = NetworkManager::AccessPoint::ApMode;
        break;
    default:
        qCDebug(NMQT) << Q_FUNC_INFO << "Unhandled mode" << mode;
    }
    return ourMode;
}

void NetworkManager::AccessPointPrivate::dbusPropertiesChanged(const QString &interfaceName,
                                                               const QVariantMap &properties,
                                                               const QStringList &invalidatedProperties)
{
    Q_UNUSED(invalidatedProperties);
    if (interfaceName == QLatin1String("org.freedesktop.NetworkManager.AccessPoint")) {
        propertiesChanged(properties);
    }
}

void NetworkManager::AccessPointPrivate::propertiesChanged(const QVariantMap &properties)
{
    Q_Q(AccessPoint);

    // qCDebug(NMQT) << Q_FUNC_INFO << properties;

    QVariantMap::const_iterator it = properties.constBegin();
    while (it != properties.constEnd()) {
        const QString property = it.key();
        if (property == QLatin1String("Flags")) {
            capabilities = convertCapabilities(it->toUInt());
            Q_EMIT q->capabilitiesChanged(capabilities);
        } else if (property == QLatin1String("WpaFlags")) {
            wpaFlags = convertWpaFlags(it->toUInt());
            Q_EMIT q->wpaFlagsChanged(wpaFlags);
        } else if (property == QLatin1String("RsnFlags")) {
            rsnFlags = convertWpaFlags(it->toUInt());
            Q_EMIT q->rsnFlagsChanged(rsnFlags);
        } else if (property == QLatin1String("Ssid")) {
            rawSsid = it->toByteArray();
            ssid = QString::fromUtf8(rawSsid);
            Q_EMIT q->ssidChanged(ssid);
        } else if (property == QLatin1String("Frequency")) {
            frequency = it->toUInt();
            Q_EMIT q->frequencyChanged(frequency);
        } else if (property == QLatin1String("HwAddress")) {
            hardwareAddress = it->toString();
        } else if (property == QLatin1String("Mode")) {
            mode = q->convertOperationMode(it->toUInt());
        } else if (property == QLatin1String("MaxBitrate")) {
            maxBitRate = it->toUInt();
            Q_EMIT q->bitRateChanged(maxBitRate);
        } else if (property == QLatin1String("Strength")) {
            signalStrength = it->toInt();
            Q_EMIT q->signalStrengthChanged(signalStrength);
        } else if (property == QLatin1String("LastSeen")) {
            lastSeen = it->toInt();
            Q_EMIT q->lastSeenChanged(lastSeen);
        } else {
            qCDebug(NMQT) << Q_FUNC_INFO << "Unhandled property" << property;
        }
        ++it;
    }
}

#include "moc_accesspoint.cpp"
#include "moc_accesspoint_p.cpp"
