// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <string>

#include "base/dbus_types.h"
#include "base/thread_pool.h"

namespace dm {

// Each user's desktop background as the login screen shows it, kept where the
// greeter can read it: the user's home may be closed to it or not mounted yet.
// <user>.conf has services.conf's [Appearance] keys, and <user>.picture is a
// copy of the picture it names.
class Backgrounds
{
public:
    // The largest picture copied, in bytes.
    static constexpr int64_t kMaximumPictureSize = 64 * 1024 * 1024;

    explicit Backgrounds(std::string directory);

    // Removes what belongs to no existing user, and copies left unfinished.
    void removeStale();

    // Copies the picture `file` reads, on a pool thread. done(false) for
    // anything but a readable regular file within kMaximumPictureSize. Of two
    // requests for one user, the later one wins whichever finishes first.
    void setPicture(const std::string &user, Lingmo::base::dbus::UnixFd file, std::function<void(bool)> done);
    // #RGB, #RRGGBB or #AARRGGBB.
    bool setColor(const std::string &user, const std::string &color);

    std::string settingsPath(const std::string &user) const;
    std::string picturePath(const std::string &user) const;

private:
    std::string m_directory;
    uint64_t m_lastRequest = 0;
    // The latest request for each user.
    std::map<std::string, uint64_t> m_latest;
    std::map<uint64_t, Lingmo::base::Job> m_copies;
};

} // namespace dm
