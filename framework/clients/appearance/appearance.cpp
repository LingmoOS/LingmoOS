// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/appearance/appearance.h"

#include <cstdint>

using namespace Lingmo::Base;

namespace Lingmo::appearance {

static constexpr const char* kService = "org.lingmo.Services";
static constexpr const char* kPath = "/org/lingmo/Services/Appearance";
static constexpr const char* kInterface = "org.lingmo.Services.Appearance";

Appearance::Appearance()
    : m_service(dbus::Connection::session(), kService, kPath, kInterface)
{
    dbus::Connection* bus = m_service.connection();
    if (!bus)
        return;
    // Read at once, so whoever shows the values has them for its first frame.
    if (const dbus::Reply reply = m_service.getAll(); !reply.isError()) {
        m_properties = reply.value<dbus::VariantMap>().value_or(dbus::VariantMap());
        m_available = true;
    }
    for (const char* member : {"darkModeChanged", "darkModeScheduleChanged", "wallpaperChanged",
                               "userWallpapersChanged", "wallpaperFoldersChanged", "continuousCornersChanged",
                               "accentColorChanged", "backgroundTypeChanged",
                               "backgroundColorChanged", "cursorThemeChanged", "cursorSizeChanged",
                               "iconThemeChanged", "systemFontPointSizeChanged", "systemFontChanged"}) {
        m_matches.push_back(m_service.onSignal(member, [this](const dbus::Message&) { reload(); }));
    }
    m_matches.push_back(m_service.onSignal("wallpaperFolderImagesChanged", [this](const dbus::Message& message) {
        wallpaperFolderImagesChanged.emit(message.argument<std::string>().value_or(std::string()));
    }));
    m_watch = bus->watchName(kService, [this](const std::string& owner) {
        const bool available = !owner.empty();
        if (available) {
            reload();
        } else if (m_available) {
            m_available = false;
            m_properties.clear();
            changed.emit();
        }
    });
}

Appearance::~Appearance() = default;

void Appearance::reload()
{
    m_reload = m_service.getAllAsync([this](const dbus::Reply& reply) {
        if (reply.isError())
            return;
        m_properties = reply.value<dbus::VariantMap>().value_or(dbus::VariantMap());
        m_available = true;
        changed.emit();
    });
}

template<typename T>
T Appearance::value(const char* name, const T& fallback) const
{
    const auto it = m_properties.find(name);
    return it == m_properties.end() ? fallback : it->second.value<T>(fallback);
}

// For a value the service announces no change of.
void Appearance::store(const char* name, dbus::Variant value)
{
    m_properties[name] = std::move(value);
    changed.emit();
}

bool Appearance::darkMode() const
{
    return value<bool>("isDarkMode", false);
}

void Appearance::setDarkMode(bool darkMode)
{
    m_service.send("setDarkMode", darkMode);
}

int Appearance::darkModeSchedule() const
{
    return value<int32_t>("darkModeSchedule", Never);
}

void Appearance::setDarkModeSchedule(int schedule)
{
    m_service.send("setDarkModeSchedule", int32_t(schedule));
}

bool Appearance::continuousCorners() const
{
    return value<bool>("continuousCorners", true);
}

void Appearance::setContinuousCorners(bool continuous)
{
    m_service.send("setContinuousCorners", continuous);
}

int Appearance::accentColor() const
{
    return value<int32_t>("accentColor", 0);
}

void Appearance::setAccentColor(int index)
{
    m_service.send("setAccentColor", int32_t(index));
}

double Appearance::fontPointSize() const
{
    return value<double>("systemFontPointSize", 0);
}

void Appearance::setFontPointSize(double size)
{
    m_service.send("setSystemFontPointSize", size);
}

std::string Appearance::fontFamily() const
{
    return value<std::string>("systemFont", {});
}

void Appearance::setFontFamily(const std::string& family)
{
    if (family.empty())
        return;
    m_service.send("setSystemFont", family);
}

std::string Appearance::fixedFontFamily() const
{
    return value<std::string>("systemFixedFont", {});
}

// The service has no signal for the fixed font, so the stored value stands.
void Appearance::setFixedFontFamily(const std::string& family)
{
    if (family.empty())
        return;
    m_service.send("setSystemFixedFont", family);
    store("systemFixedFont", family);
}

void Appearance::setFontRendering(bool antialias, const std::string& hintStyle)
{
    m_service.send("setFontRendering", antialias, hintStyle);
}

std::string Appearance::iconTheme() const
{
    return value<std::string>("iconTheme", {});
}

void Appearance::setIconTheme(const std::string& name)
{
    if (name.empty())
        return;
    m_service.send("setIconTheme", name);
}

std::string Appearance::cursorTheme() const
{
    return value<std::string>("cursorTheme", {});
}

void Appearance::setCursorTheme(const std::string& name)
{
    if (name.empty())
        return;
    m_service.send("setCursorTheme", name);
}

int Appearance::cursorSize() const
{
    return value<int32_t>("cursorSize", 24);
}

void Appearance::setCursorSize(int size)
{
    m_service.send("setCursorSize", int32_t(size));
}

std::string Appearance::wallpaper() const
{
    return value<std::string>("wallpaper", {});
}

void Appearance::setWallpaper(const std::string& path)
{
    if (path.empty())
        return;
    m_service.send("setWallpaper", path);
}

std::vector<std::string> Appearance::userWallpapers() const
{
    return value<std::vector<std::string>>("userWallpapers", {});
}

void Appearance::removeUserWallpaper(const std::string& path)
{
    if (!path.empty())
        m_service.send("removeUserWallpaper", path);
}

std::vector<std::string> Appearance::wallpaperFolders() const
{
    return value<std::vector<std::string>>("wallpaperFolders", {});
}

void Appearance::addWallpaperFolder(const std::string& path)
{
    if (!path.empty())
        m_service.send("addWallpaperFolder", path);
}

void Appearance::removeWallpaperFolder(const std::string& path)
{
    if (!path.empty())
        m_service.send("removeWallpaperFolder", path);
}

std::vector<std::string> Appearance::wallpaperFolderImages(const std::string& folder) const
{
    if (folder.empty() || !m_available)
        return {};
    return m_service.call("wallpaperFolderImages", folder).value<std::vector<std::string>>().value_or(
            std::vector<std::string>());
}

int Appearance::backgroundType() const
{
    return value<int32_t>("backgroundType", 0);
}

void Appearance::setBackgroundType(int type)
{
    m_service.send("setBackgroundType", int32_t(type));
}

std::string Appearance::backgroundColor() const
{
    return value<std::string>("backgroundColor", {});
}

void Appearance::setBackgroundColor(const std::string& color)
{
    m_service.send("setBackgroundColor", color);
}

} // namespace Lingmo::appearance
