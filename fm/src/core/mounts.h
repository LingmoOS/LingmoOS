// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"

#include "base/dbus.h"
#include "base/signal.h"

#include <functional>
#include <string>
#include <vector>

namespace fm {

// The phones, servers and disk images lingmo-mountd serves, shared by every view of the process.
class MountsClient
{
public:
    struct Mount
    {
        std::string id;
        std::string name;
        std::string icon;
        std::string path;
        bool network = false;
        bool device = false;
        bool ejectable = false;
    };

    static MountsClient& instance();

    const std::vector<Mount>& mounts() const { return m_mounts; }
    Mount mount(const std::string& id) const;
    // The mount a local path lies in, or an empty one.
    Mount mountAt(const std::string& path) const;

    using Reply = std::function<void(const std::string& status, const base::dbus::VariantMap& details)>;
    void requestMount(const std::string& location, const base::dbus::VariantMap& options, Reply reply);
    void requestUnmount(const std::string& id, std::function<void(const std::string& error)> reply);

    // smb://server/share for "server/share" or \\server\share; empty when not a location.
    static std::string normalizedAddress(const std::string& address);

    base::Signal<> mountsChanged;

private:
    MountsClient();
    void refresh();
    void call(const std::string& method, std::function<void(const base::dbus::Reply&)> done,
              std::chrono::microseconds timeout, const std::vector<base::dbus::Variant>& arguments);

    base::dbus::Proxy m_proxy;
    base::dbus::Match m_changed;
    std::vector<Mount> m_mounts;
    std::map<int, base::dbus::PendingCall> m_calls;
    int m_nextCall = 1;
};

} // namespace fm
