// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/dock/dock_settings.h"

#include <algorithm>
#include <chrono>
#include <cmath>

#include "base/settings.h"
#include "shell/common/file_thread.h"

namespace Lingmo::Shell {

static constexpr std::chrono::milliseconds kIconSizeSaveDelay(300);

DockSettings::DockSettings(std::string path)
    : m_path(std::move(path))
{
    m_iconSizeSave.setSingleShot(true);
    m_iconSizeSave.onTimeout = [this] { write("Dock/IconSize", m_iconSize); };

    m_load = runOnFileThread([path = m_path] { return load(path); },
                             [this](Values values) {
                                 const bool iconSize = values.iconSize != m_iconSize;
                                 const bool direction = values.direction != m_direction;
                                 const bool visibility = values.visibility != m_visibility;
                                 const bool tint = values.glassTint != m_glassTint;
                                 const bool area = values.minimizedArea != m_minimizedArea;
                                 const bool magnification = values.magnification != m_magnification;
                                 const bool factor =
                                         values.magnificationFactor != m_magnificationFactor;
                                 const bool bounce = values.bounceOnLaunch != m_bounceOnLaunch;
                                 const bool alignment = values.taskbarAlignment != m_taskbarAlignment;
                                 const bool search = values.taskbarSearch != m_taskbarSearch;
                                 const bool mode = !m_modeSet && values.efficientMode != m_efficientMode;
                                 if (!m_modeSet) m_efficientMode = values.efficientMode;
                                 m_iconSize = values.iconSize;
                                 m_direction = values.direction;
                                 m_visibility = values.visibility;
                                 m_edgeMargins = values.edgeMargins;
                                 m_glassTint = values.glassTint;
                                 m_minimizedArea = values.minimizedArea;
                                 m_magnification = values.magnification;
                                 m_magnificationFactor = values.magnificationFactor;
                                 m_bounceOnLaunch = values.bounceOnLaunch;
                                 m_taskbarAlignment = values.taskbarAlignment;
                                 m_taskbarSearch = values.taskbarSearch;
                                 m_ready = true;
                                 ready();
                                 if (mode) efficientModeChanged();
                                 if (iconSize)
                                     iconSizeChanged();
                                 if (direction)
                                     directionChanged();
                                 if (visibility)
                                     visibilityChanged();
                                 if (tint)
                                     glassTintChanged();
                                 if (area || mode)
                                     minimizedAreaChanged();
                                 if (magnification || mode)
                                     magnificationChanged();
                                 if (factor)
                                     magnificationFactorChanged();
                                 if (bounce || mode)
                                     bounceOnLaunchChanged();
                                 if (alignment) taskbarAlignmentChanged();
                                 if (search) taskbarSearchChanged();
                             });
}

DockSettings::Values DockSettings::load(const std::string &path)
{
    base::Settings settings(path);
    // Missing keys are written with their defaults.
    if (!settings.contains("Dock/IconSize"))
        settings.setValue("Dock/IconSize", kBaseIconSize);
    if (!settings.contains("Dock/Direction"))
        settings.setValue("Dock/Direction", int(Bottom));
    if (!settings.contains("Dock/Visibility"))
        settings.setValue("Dock/Visibility", int(AlwaysShow));
    if (!settings.contains("Dock/EdgeMargins"))
        settings.setValue("Dock/EdgeMargins", 10);
    if (!settings.contains("Dock/GlassTint"))
        settings.setValue("Dock/GlassTint", 0.5);
    if (!settings.contains("Dock/MinimizedArea"))
        settings.setValue("Dock/MinimizedArea", true);
    if (!settings.contains("Dock/Magnification"))
        settings.setValue("Dock/Magnification", true);
    if (!settings.contains("Dock/MagnificationFactor"))
        settings.setValue("Dock/MagnificationFactor", 1.5);
    if (!settings.contains("Dock/BounceOnLaunch"))
        settings.setValue("Dock/BounceOnLaunch", true);
    if (!settings.contains("Dock/TaskbarAlignment"))
        settings.setValue("Dock/TaskbarAlignment", int(AlignLeft));
    if (!settings.contains("Dock/TaskbarSearch"))
        settings.setValue("Dock/TaskbarSearch", int(SearchHidden));
    settings.sync();

    Values values;
    // Efficient mode is temporarily disabled while the taskbar/start-menu
    // implementation is being refined. Keep the persisted value intact so it
    // can be restored when the feature is enabled again.
    values.efficientMode = false;
    values.iconSize = int(settings.intValue("Dock/IconSize", kBaseIconSize));
    values.direction = Direction(settings.intValue("Dock/Direction", Bottom));
    values.visibility = Visibility(settings.intValue("Dock/Visibility", AlwaysShow));
    values.edgeMargins = int(settings.intValue("Dock/EdgeMargins", 10));
    values.glassTint = std::clamp(settings.doubleValue("Dock/GlassTint", 0.5), 0.0, 1.0);
    values.minimizedArea = settings.boolValue("Dock/MinimizedArea", true);
    values.magnification = settings.boolValue("Dock/Magnification", true);
    const double factor = settings.doubleValue("Dock/MagnificationFactor", 1.5);
    values.magnificationFactor = std::clamp(std::isfinite(factor) ? factor : 1.5, 1.0, 2.0);
    values.bounceOnLaunch = settings.boolValue("Dock/BounceOnLaunch", true);
    values.taskbarAlignment = TaskbarAlignment(settings.intValue("Dock/TaskbarAlignment", AlignLeft));
    values.taskbarSearch = TaskbarSearch(settings.intValue("Dock/TaskbarSearch", SearchHidden));
    return values;
}

void DockSettings::write(const char *key, double value)
{
    const std::string name = key;
    writeSettings(m_path, [name, value](base::Settings &settings) {
        if (name == "Dock/GlassTint" || name == "Dock/MagnificationFactor")
            settings.setValue(name, value);
        else if (name == "Dock/Magnification" || name == "Dock/BounceOnLaunch")
            settings.setValue(name, value != 0);
        else
            settings.setValue(name, int64_t(value));
    });
}

void DockSettings::setEfficientMode(bool enabled)
{
    m_modeSet = true;
    if (enabled == m_efficientMode) return;
    m_efficientMode = enabled;
    write("Desktop/Mode", enabled ? 1 : 0);
    efficientModeChanged();
    directionChanged(); iconSizeChanged(); visibilityChanged();
    magnificationChanged(); bounceOnLaunchChanged(); minimizedAreaChanged();
}

void DockSettings::setIconSize(int iconSize)
{
    if (m_iconSize == iconSize)
        return;
    m_iconSize = iconSize;
    m_iconSizeSave.start(kIconSizeSaveDelay);
    iconSizeChanged();
}

void DockSettings::setDirection(Direction direction)
{
    if (m_direction == direction)
        return;
    m_direction = direction;
    write("Dock/Direction", direction);
    directionChanged();
}

void DockSettings::setVisibility(Visibility visibility)
{
    if (m_visibility == visibility)
        return;
    m_visibility = visibility;
    write("Dock/Visibility", visibility);
    visibilityChanged();
}

void DockSettings::setGlassTint(double tint)
{
    tint = std::isfinite(tint) ? std::clamp(tint, 0.0, 1.0) : 0.5;
    if (std::abs(m_glassTint - tint) < 1e-12)
        return;
    m_glassTint = tint;
    write("Dock/GlassTint", tint);
    glassTintChanged();
}

void DockSettings::setMinimizedArea(bool enabled)
{
    if (m_minimizedArea == enabled)
        return;
    m_minimizedArea = enabled;
    writeSettings(m_path, [enabled](base::Settings &settings) { settings.setValue("Dock/MinimizedArea", enabled); });
    minimizedAreaChanged();
}

void DockSettings::setMagnification(bool enabled)
{
    if (m_magnification == enabled)
        return;
    m_magnification = enabled;
    write("Dock/Magnification", enabled ? 1 : 0);
    magnificationChanged();
}

void DockSettings::setMagnificationFactor(double factor)
{
    factor = std::isfinite(factor) ? std::clamp(factor, 1.0, 2.0) : 1.5;
    if (std::abs(m_magnificationFactor - factor) < 1e-12)
        return;
    m_magnificationFactor = factor;
    write("Dock/MagnificationFactor", factor);
    magnificationFactorChanged();
}

void DockSettings::setBounceOnLaunch(bool enabled)
{
    if (m_bounceOnLaunch == enabled)
        return;
    m_bounceOnLaunch = enabled;
    write("Dock/BounceOnLaunch", enabled ? 1 : 0);
    bounceOnLaunchChanged();
}

void DockSettings::setTaskbarAlignment(TaskbarAlignment alignment)
{
    if (alignment == m_taskbarAlignment) return;
    m_taskbarAlignment = alignment;
    write("Dock/TaskbarAlignment", alignment);
    taskbarAlignmentChanged();
}

void DockSettings::setTaskbarSearch(TaskbarSearch search)
{
    if (search == m_taskbarSearch) return;
    m_taskbarSearch = search;
    write("Dock/TaskbarSearch", search);
    taskbarSearchChanged();
}

} // namespace Lingmo::Shell
