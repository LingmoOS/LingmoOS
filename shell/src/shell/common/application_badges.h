// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <map>
#include <string>

#include "base/dbus.h"
#include "base/signal.h"

namespace Lingmo::Shell {

// The counts applications put on their icons, such as unread messages, as
// they send them in com.canonical.Unity.LauncherEntry's Update signal: the
// launcher URI application://<desktop id>.desktop and the properties that
// changed. A count shows while count-visible is true and the count is above
// zero, and goes with the connection that sent it.
class ApplicationBadges
{
public:
    // `bus` may be null: no application has a count then.
    explicit ApplicationBadges(base::dbus::Connection *bus);
    ~ApplicationBadges();

    ApplicationBadges(const ApplicationBadges &) = delete;
    ApplicationBadges &operator=(const ApplicationBadges &) = delete;

    // The count shown for a desktop id, ignoring case; 0 for none.
    int64_t count(const std::string &desktopId) const;

    // The desktop id of a launcher URI; empty for any other URI.
    static std::string desktopId(const std::string &uri);

    // A count appeared, changed or went away.
    base::Signal<> changed;

private:
    struct Entry {
        int64_t count = 0;
        bool visible = false;
        std::string sender;

        int64_t shown() const { return visible && count > 0 ? count : 0; }
    };

    void update(const std::string &sender, const std::string &uri, const base::dbus::VariantMap &properties);
    void senderVanished(const std::string &sender);

    // By lower-case desktop id.
    std::map<std::string, Entry> m_entries;
    base::dbus::Match m_update;
    base::dbus::Match m_nameOwnerChanged;
};

} // namespace Lingmo::Shell

