/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <memory>

#include "systemstats/SensorInfo.h"

class QDBusPendingCallWatcher;

namespace KSysGuard
{
/**
 * Internal helper class to communicate with the daemon.
 *
 * This is mostly for convenience on top of the auto-generated KSysGuardDaemon
 * D-Bus interface.
 */
class SensorDaemonInterface : public QObject
{
    Q_OBJECT

public:
    SensorDaemonInterface(QObject *parent = nullptr);
    ~SensorDaemonInterface() override;

    void requestMetaData(const QString &sensorId);
    void requestMetaData(const QStringList &sensorIds);
    Q_SIGNAL void metaDataChanged(const QString &sensorId, const SensorInfo &info);
    void requestValue(const QString &sensorId);
    Q_SIGNAL void valueChanged(const QString &sensorId, const QVariant &value);

    QDBusPendingCallWatcher *allSensors() const;

    void subscribe(const QString &sensorId);
    void subscribe(const QStringList &sensorIds);
    void unsubscribe(const QString &sensorId);
    void unsubscribe(const QStringList &sensorIds);

    Q_SIGNAL void sensorAdded(const QString &sensorId);
    Q_SIGNAL void sensorRemoved(const QString &sensorId);

    static SensorDaemonInterface *instance();

private:
    void onMetaDataChanged(const QHash<QString, SensorInfo> &metaData);
    void onValueChanged(const SensorDataList &values);
    void reconnect();

    class Private;
    const std::unique_ptr<Private> d;
};

}
