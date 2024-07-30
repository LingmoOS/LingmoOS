/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QDBusContext>
#include <QObject>

#include <systemstats/SensorInfo.h>

namespace KSysGuard
{
    class SensorPlugin;
    class SensorContainer;
    class SensorProperty;
}

class Client;
class QDBusServiceWatcher;

/**
 * The main central application
 */
class Daemon : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    enum class ReplaceIfRunning {
        Replace,
        DoNotReplace
    };

    Daemon();
    ~Daemon();
    bool init(ReplaceIfRunning replaceIfRunning);
    KSysGuard::SensorProperty *findSensor(const QString &path) const;

    void setQuitOnLastClientDisconnect(bool quit);

public Q_SLOTS:
    // DBus
    KSysGuard::SensorInfoMap allSensors() const;
    KSysGuard::SensorInfoMap sensors(const QStringList &sensorsIds) const;

    void subscribe(const QStringList &sensorIds);
    void unsubscribe(const QStringList &sensorIds);

    KSysGuard::SensorDataList sensorData(const QStringList &sensorIds);

Q_SIGNALS:
    // DBus
    void sensorAdded(const QString &sensorId);
    void sensorRemoved(const QString &sensorId);
    // not emitted directly as we use targetted signals via lower level API
    void newSensorData(const KSysGuard::SensorDataList &sensorData);

protected:
    // virtual for autotest to override and not load real plugins
    virtual void loadProviders();

    void sendFrame();
    void registerProvider(KSysGuard::SensorPlugin *);

private:
    void onServiceDisconnected(const QString &service);
    bool registerDBusService(const QString &serviceName, ReplaceIfRunning replace);

    QList<KSysGuard::SensorPlugin *> m_providers;
    QHash<QString /*subscriber DBus base name*/, Client*> m_clients;
    QHash<QString /*id*/, KSysGuard::SensorContainer *> m_containers;
    QDBusServiceWatcher *m_serviceWatcher;
    bool m_quitOnLastClientDisconnect = true;
};
