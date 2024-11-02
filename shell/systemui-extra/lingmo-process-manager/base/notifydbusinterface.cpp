/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "notifydbusinterface.h"

#include <QDebug>
#include <QDBusReply>

namespace {
const char *notify_service = "org.freedesktop.Notifications";
const char *notify_path = "/org/freedesktop/Notifications";
const char *notify_interface = "org.freedesktop.Notifications";
}

NotifyDBusInterface::NotifyDBusInterface(QObject *parent)
    : QObject{parent}
    , m_notifyInterface(nullptr)
{

}

void NotifyDBusInterface::initNotifyInterface()
{
    if (m_notifyInterface)
        return;

    m_notifyInterface = new QDBusInterface(notify_service, notify_path, notify_interface,
                                           QDBusConnection::sessionBus(), this);
    if (!m_notifyInterface->isValid()) {
        qWarning() << "Create Notification service interface failed:"
                   << m_notifyInterface->lastError().message();
        delete m_notifyInterface;
        m_notifyInterface = nullptr;
        return;
    } else {
        qDebug() << "Create Notification service interface successful.";
    }

    initConnections();
}

void NotifyDBusInterface::initConnections()
{
    connect(m_notifyInterface, SIGNAL(NotificationClosed(uint,uint)),
            this, SLOT(handleNotifyClose(uint,uint)));

    connect(m_notifyInterface, SIGNAL(ActionInvoked(uint,QString)),
            this, SLOT(handleActionInvoked(uint,QString)));
}

void NotifyDBusInterface::sendNotificationAsync(
    const QString &summary, const QString &body, const QStringList &actions,
    const QVariantMap &hints, uint replace, SendNotificationCallback callback)
{
    if (!m_notifyInterface) {
        initNotifyInterface();

        if (!m_notifyInterface) {
            qWarning() << "Init Notification service interface failed, can't send notification";
            return;
        }
    }

    qDebug() << "Send system notify:" << body;

    const QString app_name = QObject::tr("Settings");
    const QString app_icon = "lingmo-control-center";
    const int expire_timeout = 0;

    QDBusPendingCall call =
        m_notifyInterface->asyncCall("Notify", app_name, replace, app_icon,
                                     summary, body, actions, hints,
                                     expire_timeout);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished,
        this, [callback](QDBusPendingCallWatcher *watcher) {
            QDBusPendingReply<uint> reply = *watcher;
            if (reply.isError()) {
                qWarning() << "Send system notify failed:"
                           << reply.error().name() << reply.error().message();
                watcher->deleteLater();
                return;
            }

            if (callback) {
                callback(reply.value());
            }
            watcher->deleteLater();
        }
    );

}

void NotifyDBusInterface::setNotifyCloseCallback(NotifyCloseCallback callback)
{
    m_notifyCloseCallbacks.emplace_back(std::move(callback));
}

void NotifyDBusInterface::handleNotifyClose(uint notifyId, uint reason)
{
    for (auto &callback : m_notifyCloseCallbacks) {
        callback(notifyId, reason);
    }
}

void NotifyDBusInterface::setActionInvokedCallback(ActionInvokedCallback callback)
{
    m_actionInvokedCallbacks.emplace_back(std::move(callback));
}

void NotifyDBusInterface::handleActionInvoked(uint notifyId, const QString &actionKey)
{
    for (auto &callback : m_actionInvokedCallbacks) {
        callback(notifyId, actionKey);
    }
}
