/*
    SPDX-FileCopyrightText: 2017 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "devicestatistics_p.h"
#include "manager_p.h"
#include "nmdebug.h"

NetworkManager::DeviceStatisticsPrivate::DeviceStatisticsPrivate(const QString &path, DeviceStatistics *q)
#ifdef NMQT_STATIC
    : iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    : iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
    , refreshRateMs(0)
    , rxBytes(0)
    , txBytes(0)
    , q_ptr(q)
{
    uni = path;
}

NetworkManager::DeviceStatistics::DeviceStatistics(const QString &path, QObject *parent)
    : QObject(parent)
    , d_ptr(new DeviceStatisticsPrivate(path, this))
{
    Q_D(DeviceStatistics);

    // Refresh rate by default is 0,
    // as soon as the refresh rate is changed, we'll get the rest of properties initialised

    // Get all DeviceStatistics's properties at once
    QDBusConnection::systemBus().connect(NetworkManagerPrivate::DBUS_SERVICE,
                                         d->uni,
                                         NetworkManagerPrivate::FDO_DBUS_PROPERTIES,
                                         QLatin1String("PropertiesChanged"),
                                         d,
                                         SLOT(dbusPropertiesChanged(QString, QVariantMap, QStringList)));
}

NetworkManager::DeviceStatistics::~DeviceStatistics()
{
    Q_D(DeviceStatistics);
    delete d;
}

uint NetworkManager::DeviceStatistics::refreshRateMs() const
{
    Q_D(const DeviceStatistics);
    return d->refreshRateMs;
}

void NetworkManager::DeviceStatistics::setRefreshRateMs(uint refreshRate)
{
    Q_D(DeviceStatistics);

    // HACK calling d->iface.setRefreshRateMs does a blocking DBus call as internally it does
    // setProperty which returns whether the call succeeded, so Qt waits for it.
    // Since this can occasionally take a quite a while, this is replaced with a manual DBus call.

    QDBusMessage message = QDBusMessage::createMethodCall(NetworkManager::NetworkManagerPrivate::DBUS_SERVICE,
                                                          d->iface.path(),
                                                          NetworkManager::NetworkManagerPrivate::FDO_DBUS_PROPERTIES,
                                                          QLatin1String("Set"));
    message << d->iface.staticInterfaceName() << QLatin1String("RefreshRateMs") << QVariant::fromValue(QDBusVariant(refreshRate));

    d->iface.connection().call(message, QDBus::NoBlock);
}

qulonglong NetworkManager::DeviceStatistics::rxBytes() const
{
    Q_D(const DeviceStatistics);
    return d->rxBytes;
}

qulonglong NetworkManager::DeviceStatistics::txBytes() const
{
    Q_D(const DeviceStatistics);
    return d->txBytes;
}

void NetworkManager::DeviceStatisticsPrivate::dbusPropertiesChanged(const QString &interfaceName,
                                                                    const QVariantMap &properties,
                                                                    const QStringList &invalidatedProperties)
{
    Q_UNUSED(invalidatedProperties);
    if (interfaceName == QLatin1String("org.freedesktop.NetworkManager.Device.Statistics")) {
        propertiesChanged(properties);
    }
}

void NetworkManager::DeviceStatisticsPrivate::propertiesChanged(const QVariantMap &properties)
{
    Q_Q(DeviceStatistics);

    // qCDebug(NMQT) << Q_FUNC_INFO << properties;

    QVariantMap::const_iterator it = properties.constBegin();
    while (it != properties.constEnd()) {
        const QString property = it.key();
        if (property == QLatin1String("RefreshRateMs")) {
            refreshRateMs = it->toUInt();
            Q_EMIT q->refreshRateMsChanged(refreshRateMs);
        } else if (property == QLatin1String("RxBytes")) {
            rxBytes = it->toULongLong();
            Q_EMIT q->rxBytesChanged(rxBytes);
        } else if (property == QLatin1String("TxBytes")) {
            txBytes = it->toULongLong();
            Q_EMIT q->txBytesChanged(txBytes);
        } else {
            qCWarning(NMQT) << Q_FUNC_INFO << "Unhandled property" << property;
        }
        ++it;
    }
}

#include "moc_devicestatistics.cpp"
#include "moc_devicestatistics_p.cpp"
