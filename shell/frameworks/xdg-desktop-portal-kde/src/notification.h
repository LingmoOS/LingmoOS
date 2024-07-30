/*
 * SPDX-FileCopyrightText: 2016 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_NOTIFICATION_H
#define XDG_DESKTOP_PORTAL_KDE_NOTIFICATION_H

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

#include <KNotification>

class NotificationPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.Notification")
public:
    explicit NotificationPortal(QObject *parent);

public Q_SLOTS:
    void AddNotification(const QString &app_id, const QString &id, const QVariantMap &notification);
    void RemoveNotification(const QString &app_id, const QString &id);
private Q_SLOTS:
    void notificationClosed();

private:
    QHash<QString, QPointer<KNotification>> m_notifications;
};

#endif // XDG_DESKTOP_PORTAL_KDE_NOTIFICATION_H
