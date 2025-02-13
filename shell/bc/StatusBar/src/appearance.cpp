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

#include "appearance.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusServiceWatcher>
#include <QDBusPendingCall>
#include <QProcess>

Appearance::Appearance(QObject *parent)
    : QObject(parent)
    , m_interface("com.lingmo.Settings",
                  "/Theme",
                  "com.lingmo.Theme",
                  QDBusConnection::sessionBus())
    , m_dockSettings(new QSettings(QSettings::UserScope, "lingmoos", "dock"))
    , m_dockConfigWacher(new QFileSystemWatcher(this))
    , m_dockIconSize(0)
    , m_dockDirection(0)
    , m_fontPointSize(11)
{
    m_dockIconSize = m_dockSettings->value("IconSize").toInt();
    m_dockDirection = m_dockSettings->value("Direction").toInt();

    m_dockConfigWacher->addPath(m_dockSettings->fileName());
    connect(m_dockConfigWacher, &QFileSystemWatcher::fileChanged, this, [=] {
        m_dockSettings->sync();
        m_dockIconSize = m_dockSettings->value("IconSize").toInt();
        m_dockDirection = m_dockSettings->value("Direction").toInt();
        m_dockConfigWacher->addPath(m_dockSettings->fileName());
        emit dockIconSizeChanged();
        emit dockDirectionChanged();
    });

    // Init
    if (m_interface.isValid()) {
        m_fontPointSize = m_interface.property("systemFontPointSize").toInt();

        connect(&m_interface, SIGNAL(darkModeDimsWallpaerChanged()), this, SIGNAL(dimsWallpaperChanged()));
    }
}

void Appearance::switchDarkMode(bool darkMode)
{
    if (m_interface.isValid()) {
        m_interface.call("setDarkMode", darkMode);
    }
}

bool Appearance::dimsWallpaper() const
{
    return m_interface.property("darkModeDimsWallpaer").toBool();
}

void Appearance::setDimsWallpaper(bool value)
{
    m_interface.call("setDarkModeDimsWallpaer", value);
}

int Appearance::dockIconSize() const
{
    return m_dockIconSize;
}

void Appearance::setDockIconSize(int dockIconSize)
{
    if (m_dockIconSize == dockIconSize)
        return;

    m_dockIconSize = dockIconSize;
    m_dockSettings->setValue("IconSize", m_dockIconSize);
}

int Appearance::dockDirection() const
{
    return m_dockDirection;
}

void Appearance::setDockDirection(int dockDirection)
{
    if (m_dockDirection == dockDirection)
        return;

    m_dockDirection = dockDirection;
    m_dockSettings->setValue("Direction", m_dockDirection);
}

void Appearance::setGenericFontFamily(const QString &name)
{
    if (name.isEmpty())
        return;

    QDBusInterface iface("com.lingmo.Settings",
                         "/Theme",
                         "com.lingmo.Theme",
                         QDBusConnection::sessionBus(), this);
    if (iface.isValid()) {
        iface.call("setSystemFont", name);
    }
}

void Appearance::setFixedFontFamily(const QString &name)
{
    if (name.isEmpty())
        return;

    QDBusInterface iface("com.lingmo.Settings",
                         "/Theme",
                         "com.lingmo.Theme",
                         QDBusConnection::sessionBus(), this);
    if (iface.isValid()) {
        iface.call("setSystemFixedFont", name);
    }
}

int Appearance::fontPointSize() const
{
    return m_fontPointSize;
}

void Appearance::setFontPointSize(int fontPointSize)
{
    m_fontPointSize = fontPointSize;

    QDBusInterface iface("com.lingmo.Settings",
                         "/Theme",
                         "com.lingmo.Theme",
                         QDBusConnection::sessionBus(), this);
    if (iface.isValid()) {
        iface.call("setSystemFontPointSize", m_fontPointSize * 1.0);
    }
}

void Appearance::setAccentColor(int accentColor)
{
    QDBusInterface iface("com.lingmo.Settings",
                         "/Theme",
                         "com.lingmo.Theme",
                         QDBusConnection::sessionBus(), this);
    if (iface.isValid()) {
        iface.call("setAccentColor", accentColor);
    }
}

double Appearance::devicePixelRatio() const
{
    return m_interface.property("devicePixelRatio").toDouble();
}

void Appearance::setDevicePixelRatio(double value)
{
    QDBusInterface iface("com.lingmo.Settings",
                         "/Theme",
                         "com.lingmo.Theme",
                         QDBusConnection::sessionBus(), this);
    if (iface.isValid()) {
        iface.call("setDevicePixelRatio", value);
    }
}

void Appearance::lightThemeChanged()
{
    QProcess::startDetached("lookandfeeltool", QStringList() << "-a" << "org.lingmo.ocean.desktop");
}

void Appearance::darkThemeChanged()
{
    QProcess::startDetached("lookandfeeltool", QStringList() << "-a" << "org.lingmo.oceandark.desktop");
}