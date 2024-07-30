/*
    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PLASMA_NM_CONNECTIVITY_MONITOR_H
#define PLASMA_NM_CONNECTIVITY_MONITOR_H

#include <NetworkManagerQt/Manager>

#include <KNotification>
#include <QObject>
#include <QPointer>
#include <QTimer>

#include <QCoroCore>

class ConnectivityMonitor : public QObject
{
    Q_OBJECT
public:
    explicit ConnectivityMonitor(QObject *parent);
    ~ConnectivityMonitor() override;

private Q_SLOTS:
    void connectivityChanged(NetworkManager::Connectivity connectivity);
    QCoro::Task<void> checkConnectivity();

private:
    void showLimitedConnectivityNotification();

    QPointer<KNotification> m_notification;

    QTimer m_limitedConnectivityTimer;
};

#endif // PLASMA_NM_CONNECTIVITY_MONITOR_H
