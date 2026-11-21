// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <vector>

#include "base/dbus.h"
#include "base/file_watcher.h"
#include "base/settings.h"

namespace Lingmo::Decoration {
class Theme;

// Follows the session's dark mode, corner style, accent and font size into the
// server-side frame and window silhouettes: the shared `lingmo/theme`
// settings file, and the Appearance service's signals for the changes
// themselves, since the service writes the file lazily.
class ThemePreferences
{
public:
    explicit ThemePreferences(Theme &theme);
    ThemePreferences(const ThemePreferences &) = delete;
    ThemePreferences &operator=(const ThemePreferences &) = delete;

    void reload();
    void setDarkMode(bool dark);
    void setContinuousCorners(bool continuous);
    void setAccentColor(int index);
    void setCaptionPointSize(double points);
    // The font size signal carries no value, so it is read back from the service.
    void reloadCaptionPointSize();

private:
    Theme &m_theme;
    base::Settings m_settings;
    base::FileWatch m_watch;
    base::dbus::Proxy m_appearance;
    std::vector<base::dbus::Match> m_signals;
};

} // namespace Lingmo::Decoration
