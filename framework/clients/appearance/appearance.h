// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"

namespace Lingmo::appearance {

// The session's look, as org.lingmo.Services.Appearance keeps it: dark
// mode, accent colour, fonts, icon and cursor themes and the wallpaper. The
// values are read when the service appears and kept up to date from its
// signals; changes go to the service, which owns them, and show here once it
// reports them, so the session and its readers change together.
class Appearance
{
public:
    enum DarkModeSchedule {
        Never = 0,
        SunsetToSunrise = 1,
    };

    Appearance();
    ~Appearance();

    Appearance(const Appearance&) = delete;
    Appearance& operator=(const Appearance&) = delete;

    // Whether the service is running.
    bool isAvailable() const { return m_available; }

    bool darkMode() const;
    // While a schedule runs, this holds until its next turn.
    void setDarkMode(bool darkMode);
    int darkModeSchedule() const;
    void setDarkModeSchedule(int schedule);
    // Without the service there is nothing to read, so the default applies.
    bool continuousCorners() const;
    void setContinuousCorners(bool continuous);
    // An index into the accent colours: blue, red, green, purple, pink,
    // orange, grey.
    int accentColor() const;
    void setAccentColor(int index);

    double fontPointSize() const;
    void setFontPointSize(double size);
    std::string fontFamily() const;
    void setFontFamily(const std::string& family);
    std::string fixedFontFamily() const;
    void setFixedFontFamily(const std::string& family);
    // hintStyle: hintnone, hintslight, hintmedium or hintfull.
    void setFontRendering(bool antialias, const std::string& hintStyle);

    std::string iconTheme() const;
    void setIconTheme(const std::string& name);
    std::string cursorTheme() const;
    void setCursorTheme(const std::string& name);
    int cursorSize() const;
    void setCursorSize(int size);

    std::string wallpaper() const;
    void setWallpaper(const std::string& path);
    std::vector<std::string> userWallpapers() const;
    void removeUserWallpaper(const std::string& path);
    std::vector<std::string> wallpaperFolders() const;
    void addWallpaperFolder(const std::string& path);
    void removeWallpaperFolder(const std::string& path);
    // The service owns the copies a wallpaper folder is made of, so the list
    // is asked for rather than scanned.
    std::vector<std::string> wallpaperFolderImages(const std::string& folder) const;
    // 0 a picture, 1 a colour.
    int backgroundType() const;
    void setBackgroundType(int type);
    // "#RRGGBB".
    std::string backgroundColor() const;
    void setBackgroundColor(const std::string& color);

    // After any value changed, or the service came or went.
    base::Signal<> changed;
    // A wallpaper folder's images changed.
    base::Signal<const std::string&> wallpaperFolderImagesChanged;

private:
    void reload();
    template<typename T>
    T value(const char* name, const T& fallback) const;
    // Keeps a value the service sends no signal for.
    void store(const char* name, base::dbus::Variant value);

    base::dbus::Proxy m_service;
    base::dbus::NameWatch m_watch;
    std::vector<base::dbus::Match> m_matches;
    base::dbus::PendingCall m_reload;
    base::dbus::VariantMap m_properties;
    bool m_available = false;
};

} // namespace Lingmo::appearance
