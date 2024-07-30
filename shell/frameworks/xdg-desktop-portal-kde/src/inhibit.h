/*
 * SPDX-FileCopyrightText: 2017 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2017 Jan Grulich <jgrulich@redhat.com>
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_INHIBIT_H
#define XDG_DESKTOP_PORTAL_KDE_INHIBIT_H

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

#include "request.h"

class InhibitPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.Inhibit")
public:
    explicit InhibitPortal(QObject *parent);

public Q_SLOTS:
    void Inhibit(const QDBusObjectPath &handle, const QString &app_id, const QString &window, uint flags, const QVariantMap &options);
};

#endif // XDG_DESKTOP_PORTAL_KDE_INHIBIT_H
