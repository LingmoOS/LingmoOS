/*
 * SPDX-FileCopyrightText: 2020 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2020 Jan Grulich <jgrulich@redhat.com>
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_ACCOUNT_H
#define XDG_DESKTOP_PORTAL_KDE_ACCOUNT_H

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

class AccountPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.Account")
public:
    explicit AccountPortal(QObject *parent);

public Q_SLOTS:
    uint GetUserInformation(const QDBusObjectPath &handle, //
                            const QString &app_id,
                            const QString &parent_window,
                            const QVariantMap &options,
                            QVariantMap &results);
};

#endif // XDG_DESKTOP_PORTAL_KDE_ACCOUNT_H
