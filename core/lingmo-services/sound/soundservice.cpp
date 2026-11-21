// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "soundservice.h"

#include <algorithm>
#include <cctype>
#include <filesystem>

#include "base/paths.h"
#include "base/process.h"

namespace dbus = Lingmo::base::dbus;
namespace fs = std::filesystem;

SoundService::SoundService(Lingmo::base::Settings &settings, dbus::Connection *bus,
                           std::vector<std::string> dataRoots, Player player)
    : m_settings(settings)
    , m_bus(bus)
    , m_dataRoots(std::move(dataRoots))
    , m_player(std::move(player))
    , m_enabled(settings.boolValue("Sound/Enabled", true))
    , m_theme(settings.stringValue("Sound/Theme", "freedesktop"))
{
    if (m_dataRoots.empty()) {
        m_dataRoots.push_back(Lingmo::base::paths::dataHome());
        for (const std::string &root : Lingmo::base::paths::dataDirectories())
            m_dataRoots.push_back(root);
    }
    if (!validName(m_theme))
        m_theme = "freedesktop";
    m_deadline.setSingleShot(true);
    m_deadline.onTimeout = [this] { m_process.kill(); };
    m_process.onFinished = [this](int, bool) { m_deadline.stop(); };
    if (!m_player)
        m_player = [this](const std::string &path) {
            // Event sounds never queue or pile up. A stuck player or an
            // unexpectedly long theme file is stopped after five seconds.
            if (m_process.isRunning() || !m_process.start("pw-play", {path}))
                return false;
            m_deadline.start(std::chrono::seconds(5));
            return true;
        };
}

SoundService::~SoundService()
{
    m_deadline.stop();
    m_process.onFinished = {};
    m_process.kill();
}

bool SoundService::validName(const std::string &name)
{
    return !name.empty() && name.size() <= 80 && name.front() != '-'
            && std::all_of(name.begin(), name.end(), [](unsigned char c) {
                   return std::isalnum(c) || c == '-' || c == '_';
               });
}

std::string SoundService::soundFile(const std::string &event) const
{
    if (!validName(event))
        return {};
    // Prefer the configured theme, falling back to freedesktop. Search user
    // data before system data and stereo before a theme's flat directory.
    const auto lookup = [this](const std::string &theme, const std::string &name) {
        for (const std::string &root : m_dataRoots) {
            for (const char *variant : {"stereo", ""}) {
                for (const char *extension : {".oga", ".ogg", ".wav"}) {
                    const fs::path path = fs::path(root) / "sounds" / theme / variant / (name + extension);
                    std::error_code error;
                    if (fs::is_regular_file(path, error))
                        return path.string();
                }
            }
        }
        return std::string();
    };
    const std::string themed = lookup(m_theme, event);
    if (!themed.empty())
        return themed;
    const std::string fallback = m_theme == "freedesktop" ? std::string() : lookup("freedesktop", event);
    if (!fallback.empty())
        return fallback;
    // The default theme calls this sound camera-shutter. A theme's own
    // screen-capture file always wins above.
    if (event == "screen-capture") {
        const std::string shutter = lookup(m_theme, "camera-shutter");
        return shutter.empty() ? lookup("freedesktop", "camera-shutter") : shutter;
    }
    return {};
}

bool SoundService::playEvent(const std::string &event)
{
    if (!m_enabled || !validName(event))
        return false;
    const auto now = std::chrono::steady_clock::now();
    if (now - m_lastPlayed < std::chrono::milliseconds(150))
        return false;
    const std::string file = soundFile(event);
    if (file.empty() || !m_player(file))
        return false;
    m_lastPlayed = now;
    return true;
}

void SoundService::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;
    m_enabled = enabled;
    if (!enabled)
        m_process.kill();
    m_settings.setValue("Sound/Enabled", enabled);
    m_settings.sync();
    if (m_object)
        m_object->emitPropertiesChanged({"enabled"});
}

void SoundService::setTheme(const std::string &theme)
{
    if (!validName(theme) || theme == m_theme)
        return;
    m_theme = theme;
    m_settings.setValue("Sound/Theme", theme);
    m_settings.sync();
    if (m_object)
        m_object->emitPropertiesChanged({"theme"});
}

bool SoundService::publish()
{
    if (m_object || !m_bus)
        return bool(m_object);
    auto object = std::make_unique<dbus::Object>(m_bus, "/org/lingmo/Services/Sound",
                                                 "org.lingmo.Services.Sound");
    object->addMethod("playEvent", [this](const std::string &event) { return playEvent(event); },
                      {{"event"}, {"played"}});
    object->addMethod("setEnabled", [this](bool value) { setEnabled(value); }, {{"enabled"}, {}});
    object->addMethod("setTheme", [this](const std::string &theme) { setTheme(theme); }, {{"theme"}, {}});
    object->addProperty("enabled", [this] { return enabled(); });
    object->addProperty("theme", [this] { return theme(); });
    if (!object->publish())
        return false;
    m_object = std::move(object);
    return true;
}
