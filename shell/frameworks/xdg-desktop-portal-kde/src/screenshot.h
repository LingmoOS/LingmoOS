/*
 * SPDX-FileCopyrightText: 2018 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2018 Jan Grulich <jgrulich@redhat.com>
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_SCREENSHOT_H
#define XDG_DESKTOP_PORTAL_KDE_SCREENSHOT_H

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

class ScreenshotPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.Screenshot")
public:
    struct ColorRGB {
        double red;
        double green;
        double blue;
    };

    explicit ScreenshotPortal(QObject *parent);
    ~ScreenshotPortal() override;

public Q_SLOTS:
    uint Screenshot(const QDBusObjectPath &handle, const QString &app_id, const QString &parent_window, const QVariantMap &options, QVariantMap &results);

    uint PickColor(const QDBusObjectPath &handle, const QString &app_id, const QString &parent_window, const QVariantMap &options, QVariantMap &results);
};

#endif // XDG_DESKTOP_PORTAL_KDE_SCREENSHOT_H
