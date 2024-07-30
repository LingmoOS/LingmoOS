/*
    SPDX-FileCopyrightText: 2016 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fake_notifications_server.h"

NotificationsServer::NotificationsServer(QObject *parent)
{
    Q_UNUSED(parent);
    counter = 1;
}

uint NotificationsServer::Notify(const QString &app_name,
                                 uint replaces_id,
                                 const QString &app_icon,
                                 const QString &summary,
                                 const QString &body,
                                 const QStringList &actions,
                                 const QVariantMap &hints,
                                 int timeout)
{
    NotificationItem i;
    i.app_name = app_name;
    i.replaces_id = replaces_id;
    i.app_icon = app_icon;
    i.summary = summary;
    i.body = body;
    i.actions = actions;
    i.hints = hints;
    i.timeout = timeout;
    i.id = counter;

    notifications.append(i);

    Q_EMIT newNotification();

    return counter++;
}

void NotificationsServer::CloseNotification(uint id)
{
    QList<NotificationItem>::iterator i = notifications.begin();
    while (i != notifications.end()) {
        if ((*i).id == id) {
            notifications.erase(i);
            break;
        }
        i++;
    }

    Q_EMIT NotificationClosed(id, 3);
}

QStringList NotificationsServer::GetCapabilities()
{
    return QStringList{QStringLiteral("body-markup"), QStringLiteral("body"), QStringLiteral("actions")};
}

QString NotificationsServer::GetServerInformation(QString &vendor, QString &version, QString &specVersion)
{
    vendor = QStringLiteral("KDE");
    version = QStringLiteral("2.0"); // FIXME
    specVersion = QStringLiteral("1.1");
    return QStringLiteral("TestServer");
}

#include "moc_fake_notifications_server.cpp"
