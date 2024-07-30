/*
 * SPDX-FileCopyrightText: 2017 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2017 Jan Grulich <jgrulich@redhat.com>
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_EMAIL_H
#define XDG_DESKTOP_PORTAL_KDE_EMAIL_H

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

class EmailPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.Email")
public:
    explicit EmailPortal(QObject *parent);

public Q_SLOTS:
    uint ComposeEmail(const QDBusObjectPath &handle, const QString &app_id, const QString &window, const QVariantMap &options, QVariantMap &results);
};

#endif // XDG_DESKTOP_PORTAL_KDE_EMAIL_H
