// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/secrets/vpn_secret_fields.h"

#include <cstdint>
#include <cstdlib>

#include "base/i18n.h"

using Lingmo::i18n::tr;

namespace Lingmo::Shell {

static const std::string kServicePrefix = "org.freedesktop.NetworkManager.";

// NetworkManager's NM_SETTING_SECRET_FLAG_*.
static constexpr uint32_t kNotSaved = 0x2;
static constexpr uint32_t kNotRequired = 0x4;

static std::string shortName(const std::string &serviceType)
{
    if (serviceType.starts_with(kServicePrefix))
        return serviceType.substr(kServicePrefix.size());
    const size_t dot = serviceType.rfind('.');
    return dot == std::string::npos ? serviceType : serviceType.substr(dot + 1);
}

static std::string trimmed(const std::string &text)
{
    const size_t begin = text.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos)
        return {};
    const size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(begin, end - begin + 1);
}

std::string vpnHintMessage(const std::vector<std::string> &hints)
{
    static const std::string tag = "x-vpn-message:";
    for (const std::string &hint : hints) {
        if (hint.starts_with(tag))
            return trimmed(hint.substr(tag.size()));
    }
    return {};
}

std::vector<VpnSecretField> vpnSecretFields(const std::string &serviceType,
                                            const std::map<std::string, std::string> &data,
                                            const std::vector<std::string> &hints)
{
    const auto value = [&data](const std::string &key) {
        const auto it = data.find(key);
        return it == data.end() ? std::string() : it->second;
    };
    const auto flags = [&value](const std::string &key) {
        return uint32_t(std::strtoul(value(key + "-flags").c_str(), nullptr, 10));
    };
    const auto required = [&flags](const std::string &key) { return !(flags(key) & kNotRequired); };
    const auto has = [&data](const std::string &key) { return data.count(key) > 0; };

    const std::string password = tr("Password");
    std::vector<VpnSecretField> fields;

    // Hints name exactly what NetworkManager is missing, a one-time code for instance.
    static const std::string challenge = "x-dynamic-challenge:";
    static const std::string echoChallenge = "x-dynamic-challenge-echo:";
    const std::string message = vpnHintMessage(hints);
    for (const std::string &hint : hints) {
        if (hint.starts_with("x-vpn-message:"))
            continue;
        VpnSecretField field {hint, message.empty() ? password : message};
        if (hint.starts_with(echoChallenge)) {
            field.secret = false;
            field.label = hint.substr(echoChallenge.size());
        } else if (hint.starts_with(challenge)) {
            field.label = hint.substr(challenge.size());
        } else if (hint == "cert-pass") {
            field.label = tr("Private key password");
        } else if (hint == "http-proxy-password") {
            field.label = tr("Proxy password");
        }
        fields.push_back(field);
    }
    if (!fields.empty())
        return fields;

    const std::string kind = shortName(serviceType);
    if (kind == "openvpn") {
        const std::string type = value("connection-type");
        if ((type == "password" || type == "password-tls") && required("password"))
            fields.push_back({"password", password});
        if ((type == "tls" || type == "password-tls") && has("key") && required("cert-pass"))
            fields.push_back({"cert-pass", tr("Private key password")});
        if (has("proxy-server") && required("http-proxy-password"))
            fields.push_back({"http-proxy-password", tr("Proxy password")});
    } else if (kind == "l2tp") {
        const std::string user = value("user-auth-type");
        const std::string machine = value("machine-auth-type");
        if ((user.empty() || user == "password") && required("password"))
            fields.push_back({"password", password});
        else if (user == "tls" && required("user-certpass"))
            fields.push_back({"user-certpass", tr("Certificate password")});
        if (machine == "psk" && required("ipsec-psk"))
            fields.push_back({"ipsec-psk", tr("Pre-shared key")});
        else if (machine == "tls" && required("machine-certpass"))
            fields.push_back({"machine-certpass", tr("Machine certificate password")});
    } else if (kind == "vpnc") {
        if (required("Xauth password"))
            fields.push_back({"Xauth password", password});
        if (required("IPSec secret"))
            fields.push_back({"IPSec secret", tr("Group password")});
    } else if (kind == "strongswan") {
        const std::string method = value("method");
        if (method == "agent" || value("secret_type") == "unused")
            return fields;
        if (method == "key")
            fields.push_back({"password", tr("Private key password")});
        else if (method == "smartcard")
            fields.push_back({"password", tr("PIN")});
        else
            fields.push_back({"password", password});
    } else if (kind == "libreswan") {
        if (value("xauthpasswordinputmodes") != "unused")
            fields.push_back({"xauthpassword", password});
        if (value("pskinputmodes") != "unused")
            fields.push_back({"pskvalue", tr("Group password")});
    } else if (kind == "fortisslvpn") {
        if (required("password"))
            fields.push_back({"password", password});
        if (flags("otp") == kNotSaved)
            fields.push_back({"otp", tr("One-time password"), false});
    } else if (kind == "openconnect") {
        // Its sign-in is a conversation with the server, which no password prompt covers.
    } else if (required("password")) {
        fields.push_back({"password", password});
    }
    return fields;
}

} // namespace Lingmo::Shell
