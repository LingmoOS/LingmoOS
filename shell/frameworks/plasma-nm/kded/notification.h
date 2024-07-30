/*
    SPDX-FileCopyrightText: 2009 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_NOTIFICATION_H
#define PLASMA_NM_NOTIFICATION_H

#include <QObject>

#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/VpnConnection>

class QTimer;

class KNotification;
class Notification : public QObject
{
    Q_OBJECT
public:
    explicit Notification(QObject *parent = nullptr);

private Q_SLOTS:
    void deviceAdded(const QString &uni);
    void addDevice(const NetworkManager::Device::Ptr &device);
    void stateChanged(NetworkManager::Device::State newstate, NetworkManager::Device::State oldstate, NetworkManager::Device::StateChangeReason reason);

    void addActiveConnection(const QString &path);
    void addActiveConnection(const NetworkManager::ActiveConnection::Ptr &ac);
    void onActiveConnectionStateChanged(NetworkManager::ActiveConnection::State state);
    void onVpnConnectionStateChanged(NetworkManager::VpnConnection::State state, NetworkManager::VpnConnection::StateChangeReason reason);

    void notificationClosed();

    void onPrepareForSleep(bool sleep);
    void onCheckActiveConnectionOnResume();

private:
    QHash<QString, KNotification *> m_notifications;

    bool m_preparingForSleep = false;
    bool m_justLaunched = true;
    QStringList m_activeConnectionsBeforeSleep;
    QTimer *m_checkActiveConnectionOnResumeTimer = nullptr;
};

#endif // PLASMA_NM_NOTIFICATION_H
