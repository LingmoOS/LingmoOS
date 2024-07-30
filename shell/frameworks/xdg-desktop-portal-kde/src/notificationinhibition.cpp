/*
 * SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "notificationinhibition.h"
#include "notificationinhibition_debug.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QPointer>

static const auto s_notificationService = QStringLiteral("org.freedesktop.Notifications");
static const auto s_notificationPath = QStringLiteral("/org/freedesktop/Notifications");
static const auto s_notificationInterface = QStringLiteral("org.freedesktop.Notifications");

NotificationInhibition::NotificationInhibition(const QString &appId, const QString &reason, QObject *parent)
    : QObject(parent)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(s_notificationService, s_notificationPath, s_notificationInterface, QStringLiteral("Inhibit"));
    msg.setArguments({appId, reason, QVariantMap()});

    QPointer<NotificationInhibition> guardedThis(this);

    QDBusPendingCall pendingCall = QDBusConnection::sessionBus().asyncCall(msg);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall);
    connect(watcher, &QDBusPendingCallWatcher::finished, [guardedThis, appId, reason](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<uint> reply = *watcher;
        watcher->deleteLater();

        if (reply.isError()) {
            qCDebug(XdgDesktopPortalKdeNotificationInhibition) << "Failed to inhibit: " << reply.error().message();
            return;
        }

        const auto cookie = reply.value();

        // In case the inhibition was revoked again before the async DBus reply arrived
        if (guardedThis) {
            qCDebug(XdgDesktopPortalKdeNotificationInhibition) << "Inhibiting notifications for" << appId << "with reason" << reason << "and cookie" << cookie;
            guardedThis->m_cookie = cookie;
        } else {
            uninhibit(cookie);
        }
    });
}

NotificationInhibition::~NotificationInhibition()
{
    if (m_cookie) {
        uninhibit(m_cookie);
    }
}

void NotificationInhibition::uninhibit(uint cookie)
{
    qCDebug(XdgDesktopPortalKdeNotificationInhibition) << "Removing inhibition with cookie" << cookie;
    QDBusMessage msg = QDBusMessage::createMethodCall(s_notificationService, s_notificationPath, s_notificationInterface, QStringLiteral("UnInhibit"));
    msg.setArguments({cookie});
    QDBusConnection::sessionBus().call(msg, QDBus::NoBlock);
}
