// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/touchpad/touchpad.h"

namespace settings {

Touchpad::Touchpad()
    : m_service("org.lingmo.Touchpad1")
{
}

bool Touchpad::isAvailable() const
{
    return m_service.boolValue("available", false);
}

bool Touchpad::enabled() const
{
    return m_service.boolValue("enabled", true);
}

void Touchpad::setEnabled(bool enabled)
{
    m_service.call("setEnabled", enabled);
}

bool Touchpad::disableWithMouse() const
{
    return m_service.boolValue("disableWithMouse", false);
}

void Touchpad::setDisableWithMouse(bool enabled)
{
    m_service.call("setDisableWithMouse", enabled);
}

bool Touchpad::disableWhileTyping() const
{
    return m_service.boolValue("disableWhileTyping", true);
}

void Touchpad::setDisableWhileTyping(bool enabled)
{
    m_service.call("setDisableWhileTyping", enabled);
}

bool Touchpad::tapToClick() const
{
    return m_service.boolValue("tapToClick", false);
}

void Touchpad::setTapToClick(bool enabled)
{
    m_service.call("setTapToClick", enabled);
}

bool Touchpad::tapDragging() const
{
    return m_service.boolValue("tapDragging", false);
}

void Touchpad::setTapDragging(bool enabled)
{
    m_service.call("setTapDragging", enabled);
}

bool Touchpad::naturalScroll() const
{
    return m_service.boolValue("naturalScroll", true);
}

void Touchpad::setNaturalScroll(bool enabled)
{
    m_service.call("setNaturalScroll", enabled);
}

double Touchpad::pointerAcceleration() const
{
    return m_service.doubleValue("pointerAcceleration", 0);
}

void Touchpad::setPointerAcceleration(double value)
{
    m_service.call("setPointerAcceleration", value);
}

// The compositor's default: content under the fingers at 150%.
double Touchpad::scrollSpeed() const
{
    return m_service.doubleValue("scrollSpeed", 0.5);
}

void Touchpad::setScrollSpeed(double value)
{
    m_service.call("setScrollSpeed", value);
}

bool Touchpad::secondaryClickConfigurable() const
{
    return m_service.boolValue("secondaryClickConfigurable", false);
}

bool Touchpad::twoFingerSecondaryClick() const
{
    return m_service.boolValue("twoFingerSecondaryClick", true);
}

void Touchpad::setTwoFingerSecondaryClick(bool enabled)
{
    m_service.call("setTwoFingerSecondaryClick", enabled);
}

} // namespace settings
