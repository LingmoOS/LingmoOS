// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <string_view>

// A server address, scheme://[user[:password]@]host[:port][/path]: the scheme
// and host in lower case, an IPv6 host without its brackets, the user and the
// path percent-decoded.
struct RemoteUrl
{
    std::string scheme;
    std::string user;
    std::string password;
    std::string host;
    // -1 when none is given.
    int port = -1;
    std::string path;

    static RemoteUrl parse(std::string_view text);

    bool isValid() const
    {
        return !scheme.empty();
    }

    // scheme://user@host:port/path, the user and path percent-encoded, RFC 3986 sub-delimiters kept.
    std::string toString() const;
    // The host as it goes into another URL: an IPv6 address in brackets.
    std::string hostForUrl() const;
};
