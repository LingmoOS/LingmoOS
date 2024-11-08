/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     Lingmo OS Team <reion@lingmoos.com>
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

#include "touchpad.h"

Touchpad::Touchpad(QObject *parent)
    : QObject(parent)
    , m_iface("com.lingmo.Settings",
              "/Touchpad",
              "com.lingmo.Touchpad",
              QDBusConnection::sessionBus(), this)
{

}

bool Touchpad::available() const
{
    if (!m_iface.isValid())
        return false;

    return m_iface.property("available").toBool();
}

bool Touchpad::enabled() const
{
    return m_iface.property("enabled").toBool();
}

void Touchpad::setEnabled(bool enabled)
{
    m_iface.asyncCall("setEnabled", enabled);
    emit enabledChanged();
}

bool Touchpad::tapToClick() const
{
    return m_iface.property("tapToClick").toBool();
}

void Touchpad::setTapToClick(bool enabled)
{
    m_iface.asyncCall("setTapToClick", enabled);
    emit tapToClickChanged();
}

bool Touchpad::naturalScroll() const
{
    return m_iface.property("naturalScroll").toBool();
}

void Touchpad::setNaturalScroll(bool naturalScroll)
{
    m_iface.asyncCall("setNaturalScroll", naturalScroll);
    emit naturalScrollChanged();
}

qreal Touchpad::pointerAcceleration() const
{
    if (!m_iface.isValid())
        return 0;

    return m_iface.property("pointerAcceleration").toReal();
}

void Touchpad::setPointerAcceleration(qreal value)
{
    m_iface.asyncCall("setPointerAcceleration", value);
    emit pointerAccelerationChanged();
}
