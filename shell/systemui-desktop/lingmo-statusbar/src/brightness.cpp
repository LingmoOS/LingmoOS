/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     lingmoos <lingmoos@foxmail.com>
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

#include "brightness.h"
#include <QDBusPendingCall>

Brightness::Brightness(QObject *parent)
    : QObject(parent)
    , m_dbusConnection(QDBusConnection::sessionBus())
    , m_iface("com.lingmo.Settings",
              "/Brightness",
              "com.lingmo.Brightness", m_dbusConnection)
    , m_value(0)
    , m_enabled(false)
{
    if (!m_iface.isValid())
        return;

    m_value = m_iface.property("brightness").toInt();
    m_enabled = m_iface.property("brightnessEnabled").toBool();
}

void Brightness::setValue(int value)
{
    m_iface.asyncCall("setValue", value);
}

int Brightness::value() const
{
    return m_value;
}

bool Brightness::enabled() const
{
    return m_enabled;
}
