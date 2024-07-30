/*
    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "connectivitymonitor.h"

#include <QDBusReply>
#include <QDesktopServices>

#include <KIO/OpenUrlJob>
#include <KLocalizedString>

#include <NetworkManagerQt/ActiveConnection>

#include <QCoroCore>
#include <QCoroDBus>

#include "plasma_nm_kded.h"
#include <chrono>

using namespace std::chrono_literals;

ConnectivityMonitor::ConnectivityMonitor(QObject *parent)
    : QObject(parent)
{
    m_limitedConnectivityTimer.setSingleShot(true);
    m_limitedConnectivityTimer.setInterval(10s);
    connect(&m_limitedConnectivityTimer, &QTimer::timeout, this, &ConnectivityMonitor::showLimitedConnectivityNotification);

    checkConnectivity();

    connect(NetworkManager::notifier(), &NetworkManager::Notifier::connectivityChanged, this, &ConnectivityMonitor::connectivityChanged);
}

ConnectivityMonitor::~ConnectivityMonitor()
{
    if (m_notification) {
        m_notification->close();
    }
}

void ConnectivityMonitor::connectivityChanged(NetworkManager::Connectivity connectivity)
{
    if (m_notification && m_notification->property("nm_connectivity") != connectivity) {
        m_notification->close();
        m_notification = nullptr;
    }

    if (connectivity == NetworkManager::Limited) {
        qCDebug(PLASMA_NM_KDED_LOG) << "Network connectivity limited, scheduling notification";
        if (!m_limitedConnectivityTimer.isActive()) {
            m_limitedConnectivityTimer.start();
        }
    } else {
        m_limitedConnectivityTimer.stop();

        if (connectivity == NetworkManager::Portal) {
            qCDebug(PLASMA_NM_KDED_LOG) << "Detected captive portal";
            const NetworkManager::ActiveConnection::Ptr primaryConnection = NetworkManager::primaryConnection();
            const QString title = primaryConnection ? primaryConnection->id() : i18n("Network authentication");

            if (m_notification) {
                m_notification->setTitle(title);
                m_notification->sendEvent();
            } else {
                m_notification = new KNotification(QStringLiteral("CaptivePortal"), KNotification::Persistent);
                m_notification->setComponentName(QStringLiteral("networkmanagement"));
                m_notification->setTitle(title);
                m_notification->setText(i18n("You need to log into this network"));

                auto logInAction = m_notification->addAction(i18n("Log in"));
                connect(logInAction, &KNotificationAction::activated, this, [this]() {
                    auto job = new KIO::OpenUrlJob(QUrl(QStringLiteral("http://networkcheck.kde.org")));
                    job->setStartupId(m_notification->xdgActivationToken().toUtf8());
                    job->start();
                });
                m_notification->sendEvent();
            }
        }
    }
}

void ConnectivityMonitor::showLimitedConnectivityNotification()
{
    if (m_notification) {
        return;
    }

    m_notification = new KNotification(QStringLiteral("LimitedConnectivity"));
    m_notification->setComponentName(QStringLiteral("networkmanagement"));
    m_notification->setTitle(i18n("Limited Connectivity"));
    m_notification->setText(i18n("This device appears to be connected to a network but is unable to reach the internet."));
    m_notification->sendEvent();
}

QCoro::Task<void> ConnectivityMonitor::checkConnectivity()
{
    QDBusReply<uint> reply = co_await NetworkManager::checkConnectivity();

    if (reply.isValid()) {
        connectivityChanged((NetworkManager::Connectivity)reply.value());
    }
}

#include "moc_connectivitymonitor.cpp"
