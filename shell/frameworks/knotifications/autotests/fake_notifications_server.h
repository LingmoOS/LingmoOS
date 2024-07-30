/*
    SPDX-FileCopyrightText: 2016 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef FAKE_NOTIFICATIONS_SERVER_H
#define FAKE_NOTIFICATIONS_SERVER_H

#include <QHash>
#include <QObject>
#include <QVariantMap>

class NotificationItem
{
public:
    QString app_name;
    uint replaces_id;
    QString app_icon;
    QString summary;
    QString body;
    QStringList actions;
    QVariantMap hints;
    int timeout;
    uint id;
};

class NotificationsServer : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Notifications")

public:
    NotificationsServer(QObject *parent = nullptr);

    uint counter;
    QList<NotificationItem> notifications;

public Q_SLOTS:
    uint Notify(const QString &app_name,
                uint replaces_id,
                const QString &app_icon,
                const QString &summary,
                const QString &body,
                const QStringList &actions,
                const QVariantMap &hints,
                int timeout);

    void CloseNotification(uint id);

    QStringList GetCapabilities();

    QString GetServerInformation(QString &vendor, QString &version, QString &specVersion);

Q_SIGNALS:
    void NotificationClosed(uint id, uint reason);
    void ActionInvoked(uint id, const QString &actionKey);

    void newNotification();
};

#endif
