/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "manager_p.h"
#include "nmdebug.h"
#include "wimaxdevice.h"
#include "wimaxnsp_p.h"

namespace NetworkManager
{
NetworkManager::WimaxNsp::NetworkType convertNetworkType(uint type)
{
    switch (type) {
    case 0:
        return NetworkManager::WimaxNsp::Unknown;
    case 1:
        return NetworkManager::WimaxNsp::Home;
    case 2:
        return NetworkManager::WimaxNsp::Partner;
    case 3:
        return NetworkManager::WimaxNsp::RoamingPartner;
    }
    return NetworkManager::WimaxNsp::Unknown;
}

}

NetworkManager::WimaxNspPrivate::WimaxNspPrivate(const QString &path, WimaxNsp *q)
#ifdef NMQT_STATIC
    : iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    : iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
    , networkType(WimaxNsp::Unknown)
    , signalQuality(0)
    , q_ptr(q)
{
}

NetworkManager::WimaxNsp::WimaxNsp(const QString &path, QObject *parent)
    : QObject(parent)
    , d_ptr(new WimaxNspPrivate(path, this))
{
    Q_D(WimaxNsp);
    d->uni = path;
    if (d->iface.isValid()) {
        connect(&d->iface, &OrgFreedesktopNetworkManagerWiMaxNspInterface::PropertiesChanged, d, &WimaxNspPrivate::propertiesChanged);
        d->networkType = convertNetworkType(d->iface.networkType());
        d->name = d->iface.name();
        d->signalQuality = d->iface.signalQuality();
    }
}

NetworkManager::WimaxNsp::~WimaxNsp()
{
    Q_D(WimaxNsp);
    delete d;
}

QString NetworkManager::WimaxNsp::uni() const
{
    Q_D(const WimaxNsp);
    return d->uni;
}

NetworkManager::WimaxNsp::NetworkType NetworkManager::WimaxNsp::networkType() const
{
    Q_D(const WimaxNsp);
    return d->networkType;
}

QString NetworkManager::WimaxNsp::name() const
{
    Q_D(const WimaxNsp);
    return d->name;
}

uint NetworkManager::WimaxNsp::signalQuality() const
{
    Q_D(const WimaxNsp);
    return d->signalQuality;
}

void NetworkManager::WimaxNspPrivate::propertiesChanged(const QVariantMap &properties)
{
    Q_Q(WimaxNsp);

    QVariantMap::const_iterator it = properties.constBegin();
    while (it != properties.constEnd()) {
        const QString property = it.key();
        if (property == QLatin1String("Name")) {
            name = it->toString();
            Q_EMIT q->nameChanged(name);
        } else if (property == QLatin1String("NetworkType")) {
            networkType = convertNetworkType(it->toUInt());
            Q_EMIT q->networkTypeChanged(networkType);
        } else if (property == QLatin1String("SignalQuality")) {
            signalQuality = it->toUInt();
            Q_EMIT q->signalQualityChanged(signalQuality);
        } else {
            qCWarning(NMQT) << Q_FUNC_INFO << "Unhandled property" << property;
        }
        ++it;
    }
}

#include "moc_wimaxnsp.cpp"
#include "moc_wimaxnsp_p.cpp"
