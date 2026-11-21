// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wm/dock_preferences.h"

#include <filesystem>
#include <system_error>

#include "core/main_loop.h"
#include "core/preferences.h"
#include "wm/dock_layout_manager.h"
#include "wm/window_manager.h"

namespace Lingmo::WM {

namespace {

// The dock's DockSettings::Visibility, by value.
DockAutoHideBehavior behaviorFromSetting(int visibility)
{
    switch (visibility) {
    case 1:
        return DockAutoHideBehavior::Always;
    case 2:
        return DockAutoHideBehavior::WhenObstructed;
    default:
        return DockAutoHideBehavior::Never;
    }
}

} // namespace

DockPreferences::DockPreferences(DockLayoutManager &layout, WindowManager &manager)
    : m_layout(layout)
    , m_manager(manager)
    , m_settings(preferencesFile("shell"))
{
    const std::string directory = std::filesystem::path(m_settings.path()).parent_path();
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    // The directory is watched, so a file the dock replaces or has not
    // written yet is followed too.
    m_watch = base::FileWatch::file(
            m_settings.path(),
            [this] {
                reload();
            },
            core::mainLoop());
    reload();
}

void DockPreferences::reload()
{
    m_settings.reload();
    const bool efficient = m_settings.intValue("Desktop/Mode", 0) == 1;
    m_layout.setBehavior(efficient ? DockAutoHideBehavior::Never
                                  : behaviorFromSetting(int(m_settings.intValue("Dock/Visibility", 0))));
    m_manager.setDockTransitionsAbovePanels(!efficient && m_settings.boolValue("Dock/MinimizedArea", true));
}

} // namespace Lingmo::WM
