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

#include "multimediacontroller.h"
#include <QApplication>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QDebug>

namespace {

const char *mpris_dbus_path = "/org/mpris/MediaPlayer2";
const char *mpris_dbus_interface = "org.mpris.MediaPlayer2.Player";

}

void MultimediaController::pause(const std::string &serviceName)
{
    if (isPause(serviceName)) {
        return;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(
        QString::fromStdString(serviceName), mpris_dbus_path, mpris_dbus_interface,
        QStringLiteral("PlayPause"));

    QDBusPendingCall pcall = QDBusConnection::sessionBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, qApp,
                     [watcher, serviceName](QDBusPendingCallWatcher *call) {
                         QDBusPendingReply<> reply = *call;
                         if (reply.isError()) {
                             qWarning() << QString("Call mpris dbus %1 error: ")
                                               .arg(QString::fromStdString(serviceName));
                         }
                         call->deleteLater();
                         watcher->deleteLater();
                     });
}

bool MultimediaController::isPause(const std::string &serviceName)
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning() << "Cannot connect to the D-Bus session bus.";
        return true;
    }

    QDBusInterface mprisDbus(
        QString::fromStdString(serviceName),
        mpris_dbus_path, mpris_dbus_interface,
        QDBusConnection::sessionBus());

    if (!mprisDbus.isValid()) {
        qWarning() << QString("mpris dbus service %1 is not valid.")
                          .arg(QString::fromStdString(serviceName));
        return true;
    }

    QDBusReply<int> reply = mprisDbus.call("PlayState");
    return reply <= 0;
}
