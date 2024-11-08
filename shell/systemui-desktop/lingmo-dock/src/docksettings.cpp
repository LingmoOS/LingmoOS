/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     rekols <revenmartin@gmail.com>
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

#include "docksettings.h"

#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QDBusInterface>

#include <QFile>
#include <QDebug>

static DockSettings *SELF = nullptr;

DockSettings *DockSettings::self()
{
    if (SELF == nullptr)
        SELF = new DockSettings;

    return SELF;
}

DockSettings::DockSettings(QObject *parent)
    : QObject(parent)
    , m_iconSize(0)
    , m_edgeMargins(0)
    , m_roundedWindowEnabled(true)
    , m_direction(Left)
    , m_visibility(AlwaysShow)
    , m_settings(new QSettings(QSettings::UserScope, "lingmoos", "dock"))
{
    if (!m_settings->contains("IconSize"))
        m_settings->setValue("IconSize", 53);
    if (!m_settings->contains("Direction"))
        m_settings->setValue("Direction", Bottom);
    if (!m_settings->contains("Visibility"))
        m_settings->setValue("Visibility", AlwaysShow);
    if (!m_settings->contains("RoundedWindow"))
        m_settings->setValue("RoundedWindow", true);
    if (!m_settings->contains("Style"))
        m_settings->setValue("Style", Round);
    if (!m_settings->contains("EdgeMargins"))
        m_settings->setValue("EdgeMargins", 10);

    m_settings->sync();

    m_iconSize = m_settings->value("IconSize").toInt();
    m_direction = static_cast<Direction>(m_settings->value("Direction").toInt());
    m_visibility = static_cast<Visibility>(m_settings->value("Visibility").toInt());
    m_roundedWindowEnabled = m_settings->value("RoundedWindow").toBool();
    m_style = static_cast<Style>(m_settings->value("Style").toInt());
    m_edgeMargins = m_settings->value("EdgeMargins").toInt();
}

int DockSettings::iconSize() const
{
    return m_iconSize;
}

void DockSettings::setIconSize(int iconSize)
{
    if (m_iconSize != iconSize) {
        m_iconSize = iconSize;
        m_settings->setValue("IconSize", iconSize);
        emit iconSizeChanged();
    }
}

DockSettings::Direction DockSettings::direction() const
{
    return m_direction;
}

void DockSettings::setDirection(const Direction &direction)
{
    if (m_direction != direction) {
        m_direction = direction;
        m_settings->setValue("Direction", direction);
        emit directionChanged();
    }
}

DockSettings::Visibility DockSettings::visibility() const
{
    return m_visibility;
}

void DockSettings::setVisibility(const DockSettings::Visibility &visibility)
{
    if (m_visibility != visibility) {
        m_visibility = visibility;
        m_settings->setValue("Visibility", visibility);
        emit visibilityChanged();
    }
}

int DockSettings::edgeMargins() const
{
    return m_edgeMargins;
}

void DockSettings::setEdgeMargins(int edgeMargins)
{
    m_edgeMargins = edgeMargins;
}

bool DockSettings::roundedWindowEnabled() const
{
    return m_roundedWindowEnabled;
}

void DockSettings::setRoundedWindowEnabled(bool enabled)
{
    if (m_roundedWindowEnabled != enabled) {
        m_roundedWindowEnabled = enabled;
        emit roundedWindowEnabledChanged();
    }
}

DockSettings::Style DockSettings::style() const
{
    return m_style;
}

void DockSettings::setStyle(const DockSettings::Style &style)
{
    if (m_style != style) {
        m_style = style;
        m_settings->setValue("Style", style);
        emit styleChanged();
    }
}
