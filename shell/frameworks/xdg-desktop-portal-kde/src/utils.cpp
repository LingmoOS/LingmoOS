/*
 * SPDX-FileCopyrightText: 2018 Alexander Volkov <a.volkov@rusbitech.ru>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "utils.h"

#include "waylandintegration.h"

#include <KWindowSystem>

#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QWidget>

void Utils::setParentWindow(QWidget *w, const QString &parent_window)
{
    if (parent_window.startsWith(QLatin1String("x11:"))) {
        w->setAttribute(Qt::WA_NativeWindow, true);
        setParentWindow(w->windowHandle(), parent_window);
    }
    if (parent_window.startsWith((QLatin1String("wayland:")))) {
        if (!w->window()->windowHandle()) {
            w->window()->winId(); // create QWindow
        }
        setParentWindow(w->window()->windowHandle(), parent_window);
    }
}

void Utils::setParentWindow(QWindow *w, const QString &parent_window)
{
    if (parent_window.startsWith(QLatin1String("x11:"))) {
        KWindowSystem::setMainWindow(w, QStringView(parent_window).mid(4).toULongLong(nullptr, 16));
    }
    if (parent_window.startsWith((QLatin1String("wayland:")))) {
        WaylandIntegration::setParentWindow(w, parent_window.mid(strlen("wayland:")));
    }
}

QString Utils::applicationName(const QString &appName)
{
    QString applicationName;
    const QString desktopFile = appName + QStringLiteral(".desktop");
    const QStringList desktopFileLocations = QStandardPaths::locateAll(QStandardPaths::ApplicationsLocation, desktopFile, QStandardPaths::LocateFile);
    for (const QString &location : desktopFileLocations) {
        QSettings settings(location, QSettings::IniFormat);
        settings.beginGroup(QStringLiteral("Desktop Entry"));
        if (settings.contains(QStringLiteral("X-GNOME-FullName"))) {
            applicationName = settings.value(QStringLiteral("X-GNOME-FullName")).toString();
        } else {
            applicationName = settings.value(QStringLiteral("Name")).toString();
        }

        if (!applicationName.isEmpty()) {
            break;
        }
    }
    return applicationName;
}
