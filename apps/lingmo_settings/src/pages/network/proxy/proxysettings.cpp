// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/network/proxy/proxysettings.h"

#include <cctype>
#include <chrono>
#include <cstdlib>
#include <filesystem>

#include "base/file.h"
#include "base/paths.h"

using namespace std::chrono_literals;

namespace settings::network {

static std::string trimmed(std::string_view text)
{
    size_t begin = 0;
    size_t end = text.size();
    while (begin < end && std::isspace(static_cast<unsigned char>(text[begin])))
        ++begin;
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1])))
        --end;
    return std::string(text.substr(begin, end - begin));
}

static std::vector<std::string> readLines(const std::string& path)
{
    std::vector<std::string> lines;
    const std::optional<std::string> contents = Lingmo::base::file::read(path);
    if (!contents)
        return lines;
    size_t start = 0;
    while (start <= contents->size()) {
        const size_t end = contents->find('\n', start);
        lines.push_back(contents->substr(start, end == std::string::npos ? std::string::npos : end - start));
        if (end == std::string::npos)
            break;
        start = end + 1;
    }
    while (!lines.empty() && lines.back().empty())
        lines.pop_back();
    return lines;
}

static bool isGroupHeader(const std::string& line)
{
    return !line.empty() && line[0] == '[';
}

namespace keyfile {

std::map<std::string, Group> read(const std::string& path)
{
    std::map<std::string, Group> groups;
    Group* current = nullptr;
    for (const std::string& rawLine : readLines(path)) {
        const std::string line = trimmed(rawLine);
        if (isGroupHeader(line)) {
            const size_t end = line.find(']');
            current = end != std::string::npos && end > 0 ? &groups[line.substr(1, end - 1)] : nullptr;
            continue;
        }
        if (!current || line.starts_with('#') || line.starts_with(';'))
            continue;

        const size_t eq = line.find('=');
        if (eq != std::string::npos && eq > 0)
            (*current)[trimmed(line.substr(0, eq))] = trimmed(line.substr(eq + 1));
    }
    return groups;
}

bool writeGroup(const std::string& path, const std::string& group, const Group& values)
{
    std::vector<std::string> lines = readLines(path);
    Group pending = values;
    const std::string header = "[" + group + "]";
    bool changed = false;
    long groupEnd = -1;
    bool inGroup = false;
    bool groupFound = false;
    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string line = trimmed(lines[i]);
        if (isGroupHeader(line)) {
            if (inGroup) {
                groupEnd = long(i);
                break;
            }
            inGroup = line == header;
            groupFound = groupFound || inGroup;
            continue;
        }
        if (!inGroup)
            continue;

        const size_t eq = line.find('=');
        if (eq == std::string::npos || eq == 0)
            continue;
        const std::string key = trimmed(line.substr(0, eq));
        const auto found = pending.find(key);
        if (found == pending.end())
            continue;

        const std::string value = found->second;
        pending.erase(found);
        if (trimmed(line.substr(eq + 1)) != value) {
            lines[i] = key + "=" + value;
            changed = true;
        }
    }
    if (inGroup && groupEnd < 0)
        groupEnd = long(lines.size());

    std::vector<std::string> added;
    for (const auto& [key, value] : pending)
        added.push_back(key + "=" + value);

    if (!groupFound) {
        if (!lines.empty())
            lines.emplace_back();
        lines.push_back(header);
        lines.insert(lines.end(), added.begin(), added.end());
        changed = true;
    } else if (!added.empty()) {
        // Keep the blank separator before the next group after the inserted keys.
        long insertAt = groupEnd;
        while (insertAt > 0 && trimmed(lines[size_t(insertAt - 1)]).empty())
            --insertAt;
        lines.insert(lines.begin() + insertAt, added.begin(), added.end());
        changed = true;
    }

    if (!changed)
        return false;

    std::string contents;
    for (size_t i = 0; i < lines.size(); ++i) {
        if (i > 0)
            contents += '\n';
        contents += lines[i];
    }
    contents += '\n';
    // GLib and KConfig replace these files atomically too; a partial write would read as empty.
    return Lingmo::base::file::writeAtomically(path, contents);
}

} // namespace keyfile

static void appendUtf8(std::string& out, char32_t c)
{
    if (c < 0x80) {
        out += char(c);
    } else if (c < 0x800) {
        out += char(0xc0 | (c >> 6));
        out += char(0x80 | (c & 0x3f));
    } else if (c < 0x10000) {
        out += char(0xe0 | (c >> 12));
        out += char(0x80 | ((c >> 6) & 0x3f));
        out += char(0x80 | (c & 0x3f));
    } else {
        out += char(0xf0 | (c >> 18));
        out += char(0x80 | ((c >> 12) & 0x3f));
        out += char(0x80 | ((c >> 6) & 0x3f));
        out += char(0x80 | (c & 0x3f));
    }
}

// Parses one quoted GVariant string starting at text[pos] and moves pos past it.
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
            appendUtf8(result, char32_t(code));
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

namespace gsettings {

static std::string s_path;

std::string path()
{
    return s_path.empty() ? Lingmo::base::paths::configHome() + "/glib-2.0/settings/keyfile" : s_path;
}

void setPath(std::string path)
{
    s_path = std::move(path);
}

std::string toString(const std::string& text, const std::string& fallback)
{
    size_t pos = 0;
    std::string result;
    if (!parseString(text, pos, result))
        return fallback;
    return result;
}

int toInt(const std::string& text, int fallback)
{
    // Other writers may annotate the type, as in "uint32 8080".
    const size_t space = text.rfind(' ');
    const std::string number = space == std::string::npos ? text : text.substr(space + 1);
    char* end = nullptr;
    const long value = std::strtol(number.c_str(), &end, 10);
    return !number.empty() && end && *end == '\0' ? int(value) : fallback;
}

bool toBool(const std::string& text, bool fallback)
{
    if (text == "true")
        return true;
    if (text == "false")
        return false;
    return fallback;
}

std::vector<std::string> toStringList(const std::string& text, const std::vector<std::string>& fallback)
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

std::string stringValue(const std::string& value)
{
    std::string escaped = "'";
    for (const char c : value) {
        if (c == '\\')
            escaped += "\\\\";
        else if (c == '\'')
            escaped += "\\'";
        else if (c == '\n')
            escaped += "\\n";
        else
            escaped += c;
    }
    return escaped + "'";
}

std::string intValue(int value)
{
    return std::to_string(value);
}

} // namespace gsettings

static const std::string kProxyGroup = "system/proxy";

static std::string serverGroup(const std::string& scheme)
{
    return kProxyGroup + "/" + scheme;
}

static const keyfile::Group& groupOf(const std::map<std::string, keyfile::Group>& groups, const std::string& name)
{
    static const keyfile::Group empty;
    const auto found = groups.find(name);
    return found == groups.end() ? empty : found->second;
}

static std::string valueOf(const keyfile::Group& group, const std::string& key)
{
    const auto found = group.find(key);
    return found == group.end() ? std::string() : found->second;
}

static ProxySettings::Server loadServer(const keyfile::Group& values, int defaultPort)
{
    ProxySettings::Server server;
    server.host = gsettings::toString(valueOf(values, "host"));
    server.port = gsettings::toInt(valueOf(values, "port"), defaultPort);
    return server;
}

static void saveServer(const std::string& scheme, const ProxySettings::Server& server)
{
    keyfile::writeGroup(gsettings::path(), serverGroup(scheme),
                        {{"host", gsettings::stringValue(server.host)}, {"port", gsettings::intValue(server.port)}});
}

static std::string modeName(ProxySettings::Mode mode)
{
    switch (mode) {
    case ProxySettings::Auto:
        return "auto";
    case ProxySettings::Manual:
        return "manual";
    default:
        return "none";
    }
}

static ProxySettings::Mode modeFromName(const std::string& name)
{
    if (name == "auto")
        return ProxySettings::Auto;
    if (name == "manual")
        return ProxySettings::Manual;
    return ProxySettings::None;
}

ProxySettings ProxySettings::load()
{
    const std::map<std::string, keyfile::Group> groups = keyfile::read(gsettings::path());
    const keyfile::Group& proxy = groupOf(groups, kProxyGroup);
    const keyfile::Group& http = groupOf(groups, serverGroup("http"));

    ProxySettings settings;
    settings.mode = modeFromName(gsettings::toString(valueOf(proxy, "mode")));
    settings.autoconfigUrl = gsettings::toString(valueOf(proxy, "autoconfig-url"));
    settings.ignoreHosts = gsettings::toStringList(valueOf(proxy, "ignore-hosts"), settings.ignoreHosts);

    settings.http = loadServer(http, settings.http.port);
    settings.httpUseAuthentication = gsettings::toBool(valueOf(http, "use-authentication"), false);
    settings.httpUser = gsettings::toString(valueOf(http, "authentication-user"));
    settings.httpPassword = gsettings::toString(valueOf(http, "authentication-password"));

    settings.https = loadServer(groupOf(groups, serverGroup("https")), 0);
    settings.ftp = loadServer(groupOf(groups, serverGroup("ftp")), 0);
    settings.socks = loadServer(groupOf(groups, serverGroup("socks")), 0);
    return settings;
}

void ProxySettings::save() const
{
    // Servers first: watchers react to the mode, so it lands after what it points at.
    saveServer("http", http);
    saveServer("https", https);
    saveServer("ftp", ftp);
    saveServer("socks", socks);
    keyfile::writeGroup(gsettings::path(), kProxyGroup,
                        {{"mode", gsettings::stringValue(modeName(mode))},
                         {"autoconfig-url", gsettings::stringValue(autoconfigUrl)}});
}

static std::string portText(const ProxySettings::Server& server)
{
    return server.host.empty() || server.port <= 0 ? std::string() : std::to_string(server.port);
}

static int portValue(const std::string& text)
{
    char* end = nullptr;
    const long port = std::strtol(text.c_str(), &end, 10);
    return !text.empty() && end && *end == '\0' && port > 0 && port < 65536 ? int(port) : 0;
}

static bool sameServer(const ProxySettings& settings)
{
    return !settings.http.host.empty() && settings.ftp == settings.http;
}

static std::string percentDecoded(const std::string& text)
{
    std::string decoded;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '%' && i + 2 < text.size() && std::isxdigit(static_cast<unsigned char>(text[i + 1]))
            && std::isxdigit(static_cast<unsigned char>(text[i + 2]))) {
            decoded += char(std::strtol(text.substr(i + 1, 2).c_str(), nullptr, 16));
            i += 2;
        } else {
            decoded += text[i];
        }
    }
    return decoded;
}

NetworkProxy::NetworkProxy()
    : m_settings(ProxySettings::load())
    , m_useSameProxy(sameServer(m_settings))
{
    m_saveTimer.setSingleShot(true);
    m_saveTimer.onTimeout = [this] { m_settings.save(); };

    // gsettings replaces the file by rename; the directory is what is watched.
    m_reloadTimer.setSingleShot(true);
    m_reloadTimer.onTimeout = [this] { reload(); };
    const std::string path = gsettings::path();
    std::error_code error;
    std::filesystem::create_directories(std::filesystem::path(path).parent_path(), error);
    m_watch = Lingmo::base::FileWatch::file(path, [this] { m_reloadTimer.start(100ms); });
}

NetworkProxy::~NetworkProxy() = default;

void NetworkProxy::scheduleSave()
{
    m_saveTimer.start(0ms);
}

int NetworkProxy::flag() const
{
    switch (m_settings.mode) {
    case ProxySettings::Auto:
        return 1;
    case ProxySettings::Manual:
        return 2;
    default:
        return 0;
    }
}

void NetworkProxy::setFlag(int flag)
{
    if (this->flag() == flag)
        return;
    m_settings.mode = flag == 1 ? ProxySettings::Auto : flag == 2 ? ProxySettings::Manual : ProxySettings::None;
    scheduleSave();
    changed.emit(FlagField);
}

void NetworkProxy::setUseSameProxy(bool enabled)
{
    // Only a page state: the FTP fields it fills in are what gets saved.
    if (m_useSameProxy == enabled)
        return;
    m_useSameProxy = enabled;
    changed.emit(UseSameProxyField);
}

std::string NetworkProxy::scriptProxy() const
{
    const std::string& url = m_settings.autoconfigUrl;
    return url.starts_with("file://") ? percentDecoded(url.substr(7)) : url;
}

void NetworkProxy::setScriptProxy(const std::string& path)
{
    // The schema holds a URL; the file dialog hands over a local path.
    const std::string url = !path.empty() && path[0] == '/' ? "file://" + path : path;
    if (m_settings.autoconfigUrl == url)
        return;
    m_settings.autoconfigUrl = url;
    scheduleSave();
    changed.emit(ScriptField);
}

void NetworkProxy::setHttpProxy(const std::string& host)
{
    if (m_settings.http.host == host)
        return;
    // The page has no HTTPS field; HTTPS goes through the HTTP proxy.
    m_settings.http.host = host;
    m_settings.https.host = host;
    scheduleSave();
    changed.emit(HttpField | HttpPortField);
}

std::string NetworkProxy::httpProxyPort() const
{
    return portText(m_settings.http);
}

void NetworkProxy::setHttpProxyPort(const std::string& port)
{
    const int value = portValue(port);
    if (m_settings.http.port == value)
        return;
    m_settings.http.port = value;
    m_settings.https.port = value;
    scheduleSave();
    changed.emit(HttpPortField);
}

void NetworkProxy::setFtpProxy(const std::string& host)
{
    if (m_settings.ftp.host == host)
        return;
    m_settings.ftp.host = host;
    scheduleSave();
    changed.emit(FtpField | FtpPortField);
}

std::string NetworkProxy::ftpProxyPort() const
{
    return portText(m_settings.ftp);
}

void NetworkProxy::setFtpProxyPort(const std::string& port)
{
    const int value = portValue(port);
    if (m_settings.ftp.port == value)
        return;
    m_settings.ftp.port = value;
    scheduleSave();
    changed.emit(FtpPortField);
}

void NetworkProxy::setSocksProxy(const std::string& host)
{
    if (m_settings.socks.host == host)
        return;
    m_settings.socks.host = host;
    scheduleSave();
    changed.emit(SocksField | SocksPortField);
}

std::string NetworkProxy::socksProxyPort() const
{
    return portText(m_settings.socks);
}

void NetworkProxy::setSocksProxyPort(const std::string& port)
{
    const int value = portValue(port);
    if (m_settings.socks.port == value)
        return;
    m_settings.socks.port = value;
    scheduleSave();
    changed.emit(SocksPortField);
}

void NetworkProxy::reload()
{
    // An edit still waiting to be written would be lost; its own write triggers another reload.
    if (m_saveTimer.isActive())
        return;

    const ProxySettings previous = m_settings;
    m_settings = ProxySettings::load();
    if (m_settings == previous)
        return;

    unsigned fields = 0;
    if (m_settings.mode != previous.mode)
        fields |= FlagField;
    if (m_settings.autoconfigUrl != previous.autoconfigUrl)
        fields |= ScriptField;
    if (m_settings.http.host != previous.http.host)
        fields |= HttpField;
    if (!(m_settings.http == previous.http))
        fields |= HttpPortField;
    if (m_settings.ftp.host != previous.ftp.host)
        fields |= FtpField;
    if (!(m_settings.ftp == previous.ftp))
        fields |= FtpPortField;
    if (m_settings.socks.host != previous.socks.host)
        fields |= SocksField;
    if (!(m_settings.socks == previous.socks))
        fields |= SocksPortField;
    if (fields)
        changed.emit(fields);
    setUseSameProxy(sameServer(m_settings));
}

} // namespace settings::network
