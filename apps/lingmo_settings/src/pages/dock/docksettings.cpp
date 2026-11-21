// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/dock/docksettings.h"

#include <algorithm>
#include <cstdint>

#include "base/paths.h"
#include "base/settings.h"
#include "base/i18n.h"

using namespace Lingmo::base;

namespace settings {

DockSettings::DockSettings()
    : m_dock(dbus::Connection::session(), "org.lingmo.Dock", "/Dock", "org.lingmo.Dock")
{
    const Settings shell(paths::lingmoConfig("shell"));
    m_efficientMode = shell.intValue("Desktop/Mode", 0) == 1;
    m_dock.setTimeout(std::chrono::seconds(3));
    m_modeProperties = m_dock.onPropertiesChanged([this](const dbus::VariantMap &, const std::vector<std::string> &) { readMode(); });
    if (auto *bus = dbus::Connection::session()) {
        m_modeOwner = bus->watchName("org.lingmo.Dock", [this](const std::string &owner) {
            ++m_modeGeneration; m_modeRead = {}; m_modeWrite = {};
            m_modePending = false; m_modeAvailable = false; m_modeError.clear();
            modeChanged();
            if (!owner.empty()) readMode();
        });
    }
    m_iconSize = int(shell.intValue("Dock/IconSize"));
    m_direction = int(shell.intValue("Dock/Direction"));
    m_visibility = int(shell.intValue("Dock/Visibility"));
    m_glassTint = std::clamp(shell.doubleValue("Dock/GlassTint", 0.5), 0.0, 1.0);
    m_minimizedArea = shell.boolValue("Dock/MinimizedArea", true);
    m_magnification = shell.boolValue("Dock/Magnification", true);
    m_magnificationFactor = std::clamp(shell.doubleValue("Dock/MagnificationFactor", 1.5), 1.0, 2.0);
    m_bounceOnLaunch = shell.boolValue("Dock/BounceOnLaunch", true);
    m_taskbarAlignment = int(shell.intValue("Dock/TaskbarAlignment", 0));
    m_taskbarSearch = int(shell.intValue("Dock/TaskbarSearch", 0));
}

void DockSettings::readMode()
{
    const uint64_t generation = m_modeGeneration;
    m_modeRead = m_dock.getPropertyAsync("efficientMode", [this, generation](const dbus::Reply &reply) {
        if (generation != m_modeGeneration) return;
        const auto property = reply.value<dbus::Variant>();
        const auto value = property ? property->get<bool>() : std::optional<bool>();
        m_modeAvailable = value.has_value();
        if (value) m_efficientMode = *value;
        modeChanged();
    });
}

void DockSettings::setEfficientMode(bool enabled)
{
    if (!m_modeAvailable || m_modePending || enabled == m_efficientMode) return;
    m_modePending = true; m_modeError.clear(); modeChanged();
    const uint64_t generation = m_modeGeneration;
    m_modeWrite = m_dock.callAsync("setEfficientMode", [this, generation](const dbus::Reply &reply) {
        if (generation != m_modeGeneration) return;
        m_modePending = false;
        if (reply.isError()) m_modeError = Lingmo::I18n::tr("Could not change desktop mode.");
        modeChanged(); readMode();
    }, enabled);
}

// Called on every slider move; none of these waits for the dock.
void DockSettings::setIconSize(int size)
{
    if (size == m_iconSize)
        return;
    m_dock.send("setIconSize", int32_t(size));
    m_iconSize = size;
}

void DockSettings::setDirection(int direction)
{
    if (direction == m_direction)
        return;
    m_dock.send("setDirection", int32_t(direction));
    m_direction = direction;
}

void DockSettings::setVisibility(int visibility)
{
    if (visibility == m_visibility)
        return;
    m_dock.send("setVisibility", int32_t(visibility));
    m_visibility = visibility;
}

void DockSettings::setGlassTint(double tint)
{
    tint = std::clamp(tint, 0.0, 1.0);
    if (tint == m_glassTint)
        return;
    m_dock.send("setGlassTint", tint);
    m_glassTint = tint;
}

void DockSettings::setMinimizedArea(bool enabled)
{
    if (enabled == m_minimizedArea)
        return;
    m_dock.send("setMinimizedArea", enabled);
    m_minimizedArea = enabled;
}

void DockSettings::setMagnification(bool enabled)
{
    if (enabled == m_magnification)
        return;
    m_dock.send("setMagnification", enabled);
    m_magnification = enabled;
}

void DockSettings::setMagnificationFactor(double factor)
{
    factor = std::clamp(factor, 1.0, 2.0);
    if (factor == m_magnificationFactor)
        return;
    m_dock.send("setMagnificationFactor", factor);
    m_magnificationFactor = factor;
}

void DockSettings::setBounceOnLaunch(bool enabled)
{
    if (enabled == m_bounceOnLaunch)
        return;
    m_dock.send("setBounceOnLaunch", enabled);
    m_bounceOnLaunch = enabled;
}

void DockSettings::setTaskbarAlignment(int value)
{
    value = std::clamp(value, 0, 1);
    if (value == m_taskbarAlignment) return;
    m_dock.send("setTaskbarAlignment", int32_t(value));
    m_taskbarAlignment = value;
}

void DockSettings::setTaskbarSearch(int value)
{
    value = std::clamp(value, 0, 1);
    if (value == m_taskbarSearch) return;
    m_dock.send("setTaskbarSearch", int32_t(value));
    m_taskbarSearch = value;
}

} // namespace settings
