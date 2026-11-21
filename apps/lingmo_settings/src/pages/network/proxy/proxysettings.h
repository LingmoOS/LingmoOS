// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <map>
#include <string>
#include <vector>

#include "base/file_watcher.h"
#include "base/signal.h"
#include "base/timer.h"

namespace settings::network {

// Line-preserving access to INI-style files that several programs share, such
// as GLib's GSettings keyfile. Lines this code does not touch stay byte-identical.
namespace keyfile {

using Group = std::map<std::string, std::string>;

// Raw text after '=' for every key, by group.
std::map<std::string, Group> read(const std::string& path);
// Replaces or appends the given keys of one group. True only when the file changed.
bool writeGroup(const std::string& path, const std::string& group, const Group& values);

} // namespace keyfile

// The session runs GLib with GSETTINGS_BACKEND=keyfile: a schema at /a/b/
// lives in group [a/b] and each value is GVariant text. Missing keys mean the
// schema default.
namespace gsettings {

std::string path();
// Points path() at another file; tests keep the session's own untouched.
void setPath(std::string path);
std::string toString(const std::string& text, const std::string& fallback = {});
int toInt(const std::string& text, int fallback);
bool toBool(const std::string& text, bool fallback);
std::vector<std::string> toStringList(const std::string& text, const std::vector<std::string>& fallback);
// GVariant text for a string, an integer or a boolean.
std::string stringValue(const std::string& value);
std::string intValue(int value);

} // namespace gsettings

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
    // credentials are left as they are, since nothing in Lingmo edits them.
    void save() const;

    bool operator==(const ProxySettings&) const = default;
};

// The proxy page's view of the settings: edits are written at once, as one
// write per batch, and changes other programs make are followed while the
// page is open.
class NetworkProxy
{
public:
    enum Field : unsigned {
        FlagField = 1 << 0,
        ScriptField = 1 << 1,
        HttpField = 1 << 2,
        HttpPortField = 1 << 3,
        FtpField = 1 << 4,
        FtpPortField = 1 << 5,
        SocksField = 1 << 6,
        SocksPortField = 1 << 7,
        UseSameProxyField = 1 << 8,
    };

    NetworkProxy();
    ~NetworkProxy();

    // 0 none, 1 automatic, 2 manual.
    int flag() const;
    void setFlag(int flag);
    bool useSameProxy() const { return m_useSameProxy; }
    void setUseSameProxy(bool enabled);
    std::string scriptProxy() const;
    void setScriptProxy(const std::string& path);
    std::string httpProxy() const { return m_settings.http.host; }
    void setHttpProxy(const std::string& host);
    std::string httpProxyPort() const;
    void setHttpProxyPort(const std::string& port);
    std::string ftpProxy() const { return m_settings.ftp.host; }
    void setFtpProxy(const std::string& host);
    std::string ftpProxyPort() const;
    void setFtpProxyPort(const std::string& port);
    std::string socksProxy() const { return m_settings.socks.host; }
    void setSocksProxy(const std::string& host);
    std::string socksProxyPort() const;
    void setSocksProxyPort(const std::string& port);

    // The fields whose values changed, as Field bits.
    Lingmo::base::Signal<unsigned> changed;

private:
    void watch();
    void reload();
    void scheduleSave();

    ProxySettings m_settings;
    bool m_useSameProxy = false;
    Lingmo::base::FileWatch m_watch;
    Lingmo::base::Timer m_reloadTimer;
    // The page sets every field in a row on Save; one write covers them all.
    Lingmo::base::Timer m_saveTimer;
};

} // namespace settings::network
