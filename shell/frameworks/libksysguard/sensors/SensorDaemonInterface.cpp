/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "SensorDaemonInterface_p.h"

#include <QDBusPendingCallWatcher>

#include "systemstats/DBusInterface.h"

using namespace KSysGuard;

class SensorDaemonInterface::Private
{
public:
    std::unique_ptr<SystemStats::DBusInterface> dbusInterface;
    std::unique_ptr<QDBusServiceWatcher> serviceWatcher;
    QStringList subscribedSensors;
};

SensorDaemonInterface::SensorDaemonInterface(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    qDBusRegisterMetaType<SensorData>();
    qDBusRegisterMetaType<SensorInfo>();
    qDBusRegisterMetaType<SensorDataList>();
    qDBusRegisterMetaType<SensorInfoMap>();

    d->serviceWatcher =
        std::make_unique<QDBusServiceWatcher>(SystemStats::ServiceName, QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForUnregistration);
    connect(d->serviceWatcher.get(), &QDBusServiceWatcher::serviceUnregistered, this, &SensorDaemonInterface::reconnect);
    reconnect();
}

void KSysGuard::SensorDaemonInterface::reconnect()
{
    d->dbusInterface = std::make_unique<SystemStats::DBusInterface>();
    connect(d->dbusInterface.get(), &SystemStats::DBusInterface::sensorMetaDataChanged, this, &SensorDaemonInterface::onMetaDataChanged);
    connect(d->dbusInterface.get(), &SystemStats::DBusInterface::newSensorData, this, &SensorDaemonInterface::onValueChanged);
    connect(d->dbusInterface.get(), &SystemStats::DBusInterface::sensorAdded, this, &SensorDaemonInterface::sensorAdded);
    connect(d->dbusInterface.get(), &SystemStats::DBusInterface::sensorRemoved, this, &SensorDaemonInterface::sensorRemoved);
    subscribe(d->subscribedSensors);
}

SensorDaemonInterface::~SensorDaemonInterface()
{
}

void SensorDaemonInterface::requestMetaData(const QString &sensorId)
{
    requestMetaData(QStringList{sensorId});
}

void SensorDaemonInterface::requestMetaData(const QStringList &sensorIds)
{
    auto watcher = new QDBusPendingCallWatcher{d->dbusInterface->sensors(sensorIds), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, watcher, [this](QDBusPendingCallWatcher *self) {
        self->deleteLater();

        const QDBusPendingReply<SensorInfoMap> reply = *self;
        if (reply.isError()) {
            return;
        }

        const auto infos = reply.value();
        for (auto itr = infos.begin(); itr != infos.end(); ++itr) {
            Q_EMIT metaDataChanged(itr.key(), itr.value());
        }
    });
}

void SensorDaemonInterface::requestValue(const QString &sensorId)
{
    auto watcher = new QDBusPendingCallWatcher{d->dbusInterface->sensorData({sensorId}), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, watcher, [this](QDBusPendingCallWatcher *self) {
        self->deleteLater();

        const QDBusPendingReply<SensorDataList> reply = *self;
        if (reply.isError()) {
            return;
        }

        const auto allData = reply.value();
        for (auto data : allData) {
            Q_EMIT valueChanged(data.sensorProperty, data.payload);
        }
    });
}

QDBusPendingCallWatcher *SensorDaemonInterface::allSensors() const
{
    return new QDBusPendingCallWatcher{d->dbusInterface->allSensors()};
}

void SensorDaemonInterface::subscribe(const QString &sensorId)
{
    subscribe(QStringList{sensorId});
}

void KSysGuard::SensorDaemonInterface::subscribe(const QStringList &sensorIds)
{
    d->dbusInterface->subscribe(sensorIds);
    d->subscribedSensors.append(sensorIds);
}

void SensorDaemonInterface::unsubscribe(const QString &sensorId)
{
    unsubscribe(QStringList{sensorId});
}

void KSysGuard::SensorDaemonInterface::unsubscribe(const QStringList &sensorIds)
{
    d->dbusInterface->unsubscribe(sensorIds);
}

SensorDaemonInterface *SensorDaemonInterface::instance()
{
    static SensorDaemonInterface instance;
    return &instance;
}

void SensorDaemonInterface::onMetaDataChanged(const QHash<QString, SensorInfo> &metaData)
{
    for (auto itr = metaData.begin(); itr != metaData.end(); ++itr) {
        Q_EMIT metaDataChanged(itr.key(), itr.value());
    }
}

void SensorDaemonInterface::onValueChanged(const SensorDataList &values)
{
    for (auto entry : values) {
        Q_EMIT valueChanged(entry.sensorProperty, entry.payload);
    }
}
