// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ProxyEnvironment.h"

#include <cctype>
#include <cstdlib>

#include "base/file.h"
#include "base/paths.h"
#include "base/strings.h"
#include "base/url.h"

namespace ProxyEnvironment
{

using Group = std::map<std::string, std::string>;

static std::string trimmed(std::string_view text)
{
    return std::string(Lingmo::base::strings::trimmed(text));
}

// Raw text after '=' for every key, by group.
static std::map<std::string, Group> readKeyfile(const std::string& path)
{
    std::map<std::string, Group> groups;
    const std::optional<std::string> contents = Lingmo::base::file::read(path);
    if (!contents)
        return groups;

    Group* current = nullptr;
    size_t start = 0;
    while (start < contents->size()) {
        size_t end = contents->find('\n', start);
        if (end == std::string::npos)
            end = contents->size();
        const std::string line = trimmed(std::string_view(*contents).substr(start, end - start));
        start = end + 1;

        if (!line.empty() && line[0] == '[') {
            const size_t close = line.find(']');
            current = close != std::string::npos && close > 0 ? &groups[line.substr(1, close - 1)] : nullptr;
            continue;
        }
        if (!current || line.starts_with('#') || line.starts_with(';'))
            continue;
        const size_t eq = line.find('=');
        if (eq != std::string::npos && eq > 0)
            (*current)[trimmed(std::string_view(line).substr(0, eq))] = trimmed(std::string_view(line).substr(eq + 1));
    }
    return groups;
}

// One quoted GVariant string at text[pos]; pos moves past it.
static bool parseString(const std::string& text, size_t& pos, std::string& result)
{
    if (pos >= text.size())
        return false;
    const char quote = text[pos];
    if (quote != '\'' && quote != '"')
        return false;

    result.clear();
    for (++pos; pos < text.size(); ++pos) {
        const char c = text[pos];
        if (c == quote) {
            ++pos;
            return true;
        }
        if (c != '\\') {
            result += c;
            continue;
        }
        if (++pos >= text.size())
            return false;

        const char escaped = text[pos];
        switch (escaped) {
        case 'n': result += '\n'; break;
        case 't': result += '\t'; break;
        case 'r': result += '\r'; break;
        case 'b': result += '\b'; break;
        case 'f': result += '\f'; break;
        case 'v': result += '\v'; break;
        case 'a': result += '\a'; break;
        case 'u':
        case 'U': {
            const size_t digits = escaped == 'u' ? 4 : 8;
            const std::string hex = text.substr(pos + 1, digits);
            char* end = nullptr;
            const unsigned long code = std::strtoul(hex.c_str(), &end, 16);
            if (hex.size() != digits || !end || *end != '\0')
                return false;
            Lingmo::base::strings::appendUtf8(result, char32_t(code));
            pos += digits;
            break;
        }
        default:
            result += escaped;
        }
    }
    return false;
}

static void skipSpaces(const std::string& text, size_t& pos)
{
    while (pos < text.size() && std::isspace(static_cast<unsigned char>(text[pos])))
        ++pos;
}

static std::string toString(const std::string& text)
{
    size_t pos = 0;
    std::string result;
    if (!parseString(text, pos, result))
        return std::string();
    return result;
}

static int toInt(const std::string& text, int fallback)
{
    // Other writers may annotate the type, as in "uint32 8080".
    const size_t space = text.rfind(' ');
    const std::string number = space == std::string::npos ? text : text.substr(space + 1);
    char* end = nullptr;
    const long value = std::strtol(number.c_str(), &end, 10);
    return !number.empty() && end && *end == '\0' ? int(value) : fallback;
}

static bool toBool(const std::string& text, bool fallback)
{
    if (text == "true")
        return true;
    if (text == "false")
        return false;
    return fallback;
}

static std::vector<std::string> toStringList(const std::string& text, const std::vector<std::string>& fallback)
{
    std::string list = text;
    if (list.starts_with("@as"))
        list = trimmed(list.substr(3));
    if (!list.starts_with('['))
        return fallback;

    std::vector<std::string> result;
    size_t pos = 1;
    skipSpaces(list, pos);
    if (pos < list.size() && list[pos] == ']')
        return result;

    while (pos < list.size()) {
        std::string item;
        if (!parseString(list, pos, item))
            return fallback;
        result.push_back(item);

        skipSpaces(list, pos);
        if (pos >= list.size())
            return fallback;
        if (list[pos] == ']')
            return result;
        if (list[pos] != ',')
            return fallback;
        ++pos;
        skipSpaces(list, pos);
    }
    return fallback;
}

struct Server
{
    std::string host;
    int port = 0;

    bool isValid() const { return !host.empty() && port > 0 && port < 65536; }
    bool operator==(const Server&) const = default;
};

static std::string valueOf(const std::map<std::string, Group>& groups, const std::string& group, const std::string& key)
{
    const auto foundGroup = groups.find(group);
    if (foundGroup == groups.end())
        return std::string();
    const auto found = foundGroup->second.find(key);
    return found == foundGroup->second.end() ? std::string() : found->second;
}

static Server loadServer(const std::map<std::string, Group>& groups, const std::string& scheme, int defaultPort)
{
    const std::string group = "system/proxy/" + scheme;
    return Server{toString(valueOf(groups, group, "host")), toInt(valueOf(groups, group, "port"), defaultPort)};
}

static std::string hostPort(const Server& server)
{
    std::string host = server.host;
    if (host.find(':') != std::string::npos && !host.starts_with('['))
        host = "[" + host + "]";
    return host + ":" + std::to_string(server.port);
}

std::vector<std::string> names()
{
    std::vector<std::string> result;
    for (const char* name : {"http_proxy", "https_proxy", "ftp_proxy", "all_proxy", "no_proxy"}) {
        result.push_back(name);
        result.push_back(Lingmo::base::strings::toAsciiUpper(name));
    }
    return result;
}

std::map<std::string, std::string> variables()
{
    const std::map<std::string, Group> groups = readKeyfile(Lingmo::base::paths::configHome()
                                                            + "/glib-2.0/settings/keyfile");
    std::map<std::string, std::string> lower;

    // A PAC script cannot be expressed as variables, so auto mode starts programs direct.
    if (toString(valueOf(groups, "system/proxy", "mode")) != "manual")
        return lower;

    const std::vector<std::string> ignoreHosts = toStringList(valueOf(groups, "system/proxy", "ignore-hosts"),
                                                              {"localhost", "127.0.0.0/8", "::1"});
    const Server http = loadServer(groups, "http", 8080);
    const bool useAuthentication = toBool(valueOf(groups, "system/proxy/http", "use-authentication"), false);
    const std::string user = toString(valueOf(groups, "system/proxy/http", "authentication-user"));
    const std::string password = toString(valueOf(groups, "system/proxy/http", "authentication-password"));
    const Server httpsServer = loadServer(groups, "https", 0);
    const Server ftp = loadServer(groups, "ftp", 0);
    const Server socks = loadServer(groups, "socks", 0);

    std::string credentials;
    if (useAuthentication && !user.empty()) {
        credentials = Lingmo::base::url::percentEncoded(user);
        if (!password.empty())
            credentials += ":" + Lingmo::base::url::percentEncoded(password);
        credentials += "@";
    }

    if (http.isValid())
        lower["http_proxy"] = "http://" + credentials + hostPort(http);

    // GIO's reading of the schema: HTTPS falls back to the HTTP proxy.
    const Server https = httpsServer.host.empty() ? http : httpsServer;
    if (https.isValid())
        lower["https_proxy"] = "http://" + (https == http ? credentials : std::string()) + hostPort(https);

    if (ftp.isValid())
        lower["ftp_proxy"] = "http://" + hostPort(ftp);

    // curl reads a bare socks:// as SOCKS4; proxy clients all speak SOCKS5.
    if (socks.isValid())
        lower["all_proxy"] = "socks5://" + hostPort(socks);

    if (lower.empty())
        return lower;

    std::vector<std::string> noProxy;
    for (const std::string& host : ignoreHosts) {
        // GIO's "*.example.com" is ".example.com" to curl and Go.
        noProxy.push_back(host.starts_with("*.") ? host.substr(1) : host);
    }
    if (!noProxy.empty())
        lower["no_proxy"] = Lingmo::base::strings::join(noProxy, ",");

    std::map<std::string, std::string> result;
    for (const auto& [name, value] : lower) {
        result[name] = value;
        result[Lingmo::base::strings::toAsciiUpper(name)] = value;
    }
    return result;
}

}
