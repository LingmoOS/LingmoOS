/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QObject>

#include <systemstats/SensorInfo.h>

namespace KSysGuard
{
    class SensorProperty;
}
class Daemon;

/**
 * This class represents an individual connection to the daemon
 */
class Client : public QObject
{
    Q_OBJECT
public:
    Client(Daemon *parent, const QString &serviceName);
    ~Client() override;
    void subscribeSensors(const QStringList &sensorIds);
    void unsubscribeSensors(const QStringList &sensorIds);
    void sendFrame();

private:
    void sendValues(const KSysGuard::SensorDataList &updates);
    void sendMetaDataChanged(const KSysGuard::SensorInfoMap &sensors);

    const QString m_serviceName;
    Daemon *m_daemon;
    QHash<QString, KSysGuard::SensorProperty *> m_subscribedSensors;
    QMultiHash<KSysGuard::SensorProperty *, QMetaObject::Connection> m_connections;
    KSysGuard::SensorDataList m_pendingUpdates;
    KSysGuard::SensorInfoMap m_pendingMetaDataChanges;
};
