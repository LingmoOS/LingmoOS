/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "dbushelper.h"

#include <QDBusError>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

namespace
{
bool isFakeEnv()
{
    return qEnvironmentVariableIsSet("KBOLT_FAKE");
}

} // namespace

QDBusConnection DBusHelper::connection()
{
    if (isFakeEnv()) {
        return QDBusConnection::sessionBus();
    } else {
        return QDBusConnection::systemBus();
    }
}

QString DBusHelper::serviceName()
{
    if (isFakeEnv()) {
        return QStringLiteral("org.kde.fakebolt");
    } else {
        return QStringLiteral("org.freedesktop.bolt");
    }
}

void DBusHelper::handleCall(QDBusPendingCall call, CallOkCallback &&okCb, CallErrorCallback &&errCb, QObject *parent)
{
    auto watcher = new QDBusPendingCallWatcher(call);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, parent, [okCb = std::move(okCb), errCb = std::move(errCb)](QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();
        const QDBusPendingReply<void> reply(*watcher);
        if (reply.isError()) {
            if (errCb) {
                errCb(reply.error().message());
            }
        } else if (okCb) {
            okCb();
        }
    });
}
