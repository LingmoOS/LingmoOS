// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_WM_DOCK_PREFERENCES_H_
#define LINGMO_WM_DOCK_PREFERENCES_H_

#include "base/file_watcher.h"
#include "base/settings.h"

namespace Lingmo::WM {

class DockLayoutManager;
class WindowManager;

// Follows the dock's visibility and minimized area settings into the dock
// layout and the stacking: the shared settings file the dock writes when the
// user changes them.
class DockPreferences
{
public:
    DockPreferences(DockLayoutManager &layout, WindowManager &manager);

    void reload();

private:
    DockLayoutManager &m_layout;
    WindowManager &m_manager;
    base::Settings m_settings;
    base::FileWatch m_watch;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_DOCK_PREFERENCES_H_
