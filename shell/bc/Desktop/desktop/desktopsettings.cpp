/*
 * Copyright (C) 2025 Lingmo OS Team.
 *
 * Author:     Lingmo OS Team <team@lingmo.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "desktopsettings.h"

#include <QDBusServiceWatcher>
#include <QProcess>
#include <QSettings>

DesktopSettings::DesktopSettings(QObject *parent)
    : QObject(parent)
    , m_interface("com.lingmo.Settings",
                  "/Theme", "com.lingmo.Theme",
                  QDBusConnection::sessionBus(), this)
{
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(this);
    watcher->setConnection(QDBusConnection::sessionBus());
    watcher->addWatchedService("com.lingmo.Settings");
    connect(watcher, &QDBusServiceWatcher::serviceRegistered, this, &DesktopSettings::init);
    QSettings settings("/etc/os-release",QSettings::IniFormat);
    m_currentVersion = settings.value("PRETTY_NAME").toString();

    init();
}

QString DesktopSettings::wallpaper() const
{
    return m_wallpaper;
}

bool DesktopSettings::backgroundVisible() const
{
    return m_interface.property("backgroundVisible").toBool();
}

bool DesktopSettings::dimsWallpaper() const
{
    return m_interface.property("darkModeDimsWallpaer").toBool();
}

int DesktopSettings::backgroundType() const
{
    return m_interface.property("backgroundType").toInt();
}

QString DesktopSettings::backgroundColor() const
{
    return m_interface.property("backgroundColor").toString();
}

void DesktopSettings::launch(const QString &command, const QStringList &args)
{
    QProcess process;
    process.setProgram(command);
    process.setArguments(args);
    process.startDetached();
}

void DesktopSettings::init()
{
    if (m_interface.isValid()) {
        connect(&m_interface, SIGNAL(wallpaperChanged(QString)), this, SLOT(onWallpaperChanged(QString)));
        connect(&m_interface, SIGNAL(darkModeDimsWallpaerChanged()), this, SIGNAL(dimsWallpaperChanged()));
        connect(&m_interface, SIGNAL(backgroundTypeChanged()), this, SIGNAL(backgroundTypeChanged()));
        connect(&m_interface, SIGNAL(backgroundColorChanged()), this, SIGNAL(backgroundColorChanged()));
        connect(&m_interface, SIGNAL(backgroundVisibleChanged()), this, SIGNAL(backgroundVisibleChanged()));
        m_wallpaper = m_interface.property("wallpaper").toString();
        emit wallpaperChanged();
    }
}

void DesktopSettings::onWallpaperChanged(QString path)
{
    if (path != m_wallpaper) {
        m_wallpaper = path;
        emit wallpaperChanged();
    }
}

QString DesktopSettings::version()
{
    return m_currentVersion;
}