// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace Lingmo::Files {

// A URL: the scheme in lower case and the other parts decoded, so path() of
// "file:///a%20b" is "/a b". toString() writes it percent-encoded again.
class Url
{
public:
    Url() = default;
    // Parses an encoded URL, such as "file:///tmp/a%20b" or "trash:/home/a";
    // without a scheme it is a relative URL holding only a path.
    explicit Url(std::string_view text);

    static Url fromLocalFile(std::string_view path);

    // A URL with no scheme, authority or path is empty and not valid.
    bool isValid() const { return m_valid; }
    bool isEmpty() const;

    const std::string& scheme() const { return m_scheme; }
    const std::string& userInfo() const { return m_userInfo; }
    const std::string& host() const { return m_host; }
    // -1 when none is given.
    int port() const { return m_port; }
    const std::string& path() const { return m_path; }
    bool hasQuery() const { return m_hasQuery; }
    const std::string& query() const { return m_query; }
    bool hasFragment() const { return m_hasFragment; }
    const std::string& fragment() const { return m_fragment; }

    void setScheme(std::string scheme);
    // Taken as it is, already decoded.
    void setPath(std::string path);

    bool isLocalFile() const { return m_scheme == "file"; }
    // The path of a file: URL, with "//host" before it for a remote host;
    // empty for another scheme.
    std::string toLocalFile() const;
    // What follows the path's last slash.
    std::string fileName() const;

    // The URL with the path's trailing slashes removed, "/" kept.
    Url strippedTrailingSlash() const;
    // The folder the URL's file is in: the path up to its last slash, without
    // trailing slashes.
    Url folder() const;

    std::string toString() const;

    bool operator==(const Url& other) const;
    bool operator<(const Url& other) const { return toString() < other.toString(); }

private:
    std::string m_scheme;
    std::string m_userInfo;
    std::string m_host;
    std::string m_path;
    std::string m_query;
    std::string m_fragment;
    int m_port = -1;
    bool m_hasAuthority = false;
    bool m_hasQuery = false;
    bool m_hasFragment = false;
    bool m_valid = false;
};

// The parent folder's URL.
Url upUrl(const Url& url);

struct UrlHash
{
    size_t operator()(const Url& url) const;
};

} // namespace Lingmo::Files
