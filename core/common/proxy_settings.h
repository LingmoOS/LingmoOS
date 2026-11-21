// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <map>
#include <string>
#include <vector>

#include "common/environment.h"

namespace Lingmo::Core {

// The session proxy, stored where GNOME keeps it: org.gnome.system.proxy in
// the GSettings keyfile. Third-party proxy clients write it with gsettings, so
// every other consumer derives from it.
struct ProxySettings
{
    enum Mode {
        None,
        Auto,
        Manual,
    };

    struct Server
    {
        std::string host;
        int port = 0;

        bool isValid() const { return !host.empty() && port > 0 && port < 65536; }
        bool operator==(const Server&) const = default;
    };

    Mode mode = None;
    std::string autoconfigUrl;
    std::vector<std::string> ignoreHosts{"localhost", "127.0.0.0/8", "::1"};
    Server http{{}, 8080};
    Server https;
    Server ftp;
    Server socks;
    bool httpUseAuthentication = false;
    std::string httpUser;
    std::string httpPassword;

    static ProxySettings load();
    // Writes the mode, the PAC URL and the servers. Ignore hosts and
    // credentials are left as they are, since nothing in Cutefish edits them.
    void save() const;

    // GIO's reading of the schema: HTTPS falls back to the HTTP proxy, other schemes to SOCKS.
    Server effectiveHttps() const;

    bool operator==(const ProxySettings&) const = default;
};

namespace proxy {

// Every variable the proxy owns; a launch drops the ones the current mode does not set.
std::vector<std::string> names();
std::map<std::string, std::string> variables(const ProxySettings& settings);
void apply(Environment& environment, const ProxySettings& settings);
void apply(Environment& environment);

} // namespace proxy

} // namespace Lingmo::Core
