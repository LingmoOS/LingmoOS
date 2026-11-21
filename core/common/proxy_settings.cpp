// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common/proxy_settings.h"

#include <cctype>

#include "base/url.h"
#include "common/gsettings_keyfile.h"

namespace Lingmo::Core {

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

ProxySettings::Server ProxySettings::effectiveHttps() const
{
    return https.host.empty() ? http : https;
}

namespace proxy {

static std::string hostPort(const ProxySettings::Server& server)
{
    std::string host = server.host;
    if (host.find(':') != std::string::npos && !host.starts_with('['))
        host = '[' + host + ']';
    return host + ':' + std::to_string(server.port);
}

static std::string toUpper(std::string text)
{
    for (char& c : text)
        c = char(std::toupper(static_cast<unsigned char>(c)));
    return text;
}

std::vector<std::string> names()
{
    static const char* const lower[] = {"http_proxy", "https_proxy", "ftp_proxy", "all_proxy", "no_proxy"};
    std::vector<std::string> result;
    for (const char* name : lower) {
        result.push_back(name);
        result.push_back(toUpper(name));
    }
    return result;
}

std::map<std::string, std::string> variables(const ProxySettings& settings)
{
    std::map<std::string, std::string> lower;
    // A PAC script cannot be expressed as variables, so auto mode starts applications direct.
    if (settings.mode != ProxySettings::Manual)
        return lower;

    std::string credentials;
    if (settings.httpUseAuthentication && !settings.httpUser.empty()) {
        credentials = Lingmo::Base::url::percentEncoded(settings.httpUser);
        if (!settings.httpPassword.empty())
            credentials += ':' + Lingmo::Base::url::percentEncoded(settings.httpPassword);
        credentials += '@';
    }

    if (settings.http.isValid())
        lower["http_proxy"] = "http://" + credentials + hostPort(settings.http);

    const ProxySettings::Server https = settings.effectiveHttps();
    if (https.isValid()) {
        const std::string httpsCredentials = https == settings.http ? credentials : std::string();
        lower["https_proxy"] = "http://" + httpsCredentials + hostPort(https);
    }

    if (settings.ftp.isValid())
        lower["ftp_proxy"] = "http://" + hostPort(settings.ftp);

    // curl reads a bare socks:// as SOCKS4; proxy clients all speak SOCKS5.
    if (settings.socks.isValid())
        lower["all_proxy"] = "socks5://" + hostPort(settings.socks);

    if (lower.empty())
        return lower;

    std::string noProxy;
    for (const std::string& host : settings.ignoreHosts) {
        if (!noProxy.empty())
            noProxy += ',';
        // GIO's "*.example.com" is ".example.com" to curl and Go.
        noProxy += host.starts_with("*.") ? host.substr(1) : host;
    }
    if (!settings.ignoreHosts.empty())
        lower["no_proxy"] = noProxy;

    std::map<std::string, std::string> result;
    for (const auto& [name, value] : lower) {
        result[name] = value;
        result[toUpper(name)] = value;
    }
    return result;
}

void apply(Environment& environment, const ProxySettings& settings)
{
    for (const std::string& name : names())
        environment.erase(name);

    for (const auto& [name, value] : variables(settings))
        environment[name] = value;
}

void apply(Environment& environment)
{
    proxy::apply(environment, ProxySettings::load());
}

} // namespace proxy

} // namespace Lingmo::Core
