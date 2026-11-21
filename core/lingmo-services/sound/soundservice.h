// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/settings.h"
#include "base/process.h"
#include "base/timer.h"

// Session-wide event sounds. Only theme event names, never arbitrary paths,
// cross the D-Bus boundary. Settings and playback live in lingmo-services.
class SoundService
{
public:
    using Player = std::function<bool(const std::string &path)>;

    SoundService(Lingmo::base::Settings &settings, Lingmo::base::dbus::Connection *bus,
                 std::vector<std::string> dataRoots = {}, Player player = {});
    ~SoundService();
    bool publish();

    bool enabled() const { return m_enabled; }
    void setEnabled(bool enabled);
    const std::string &theme() const { return m_theme; }
    void setTheme(const std::string &theme);
    // Returns true if a themed file was accepted by the player.
    bool playEvent(const std::string &event);
    std::string soundFile(const std::string &event) const;

private:
    static bool validName(const std::string &name);

    Lingmo::base::Settings &m_settings;
    Lingmo::base::dbus::Connection *m_bus;
    std::vector<std::string> m_dataRoots;
    Player m_player;
    bool m_enabled;
    std::string m_theme;
    std::chrono::steady_clock::time_point m_lastPlayed{};
    Lingmo::base::Process m_process;
    Lingmo::base::Timer m_deadline;
    std::unique_ptr<Lingmo::base::dbus::Object> m_object;
};
