// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "lingmo/networkmanager/types.h"
#include "lingmo/networkmanager/vpn_schema.h"

using namespace Lingmo::Networkmanager;
namespace schema = Lingmo::networkmanager::vpn::schema;
using Strings = std::map<std::string, std::string>;

static schema::FormField formItem(const std::string& kind, const Strings& data, const std::string& key)
{
    for (const schema::FormField& item : schema::formFields(kind, data)) {
        if (item.key == key)
            return item;
    }
    return {};
}

static std::string get(const Strings& map, const std::string& key)
{
    const auto found = map.find(key);
    return found == map.end() ? std::string() : found->second;
}

TEST(VpnSchema, EveryKindHasAServerField)
{
    EXPECT_EQ(schema::kinds().size(), 11u);
    for (const std::string& kind : schema::kinds()) {
        const std::vector<schema::Field> fields = schema::fields(kind);
        EXPECT_FALSE(fields.empty()) << kind;
        bool required = false;
        for (const schema::Field& field : fields) {
            required |= field.required;
            EXPECT_TRUE(!field.key.empty() && !field.label.empty()) << kind;
            if (field.type == schema::Type::Choice)
                EXPECT_GE(field.choices.size(), 2u) << kind << "/" << field.key;
        }
        EXPECT_TRUE(required) << kind;
    }
    EXPECT_TRUE(schema::fields("wireguard").empty());
    EXPECT_TRUE(schema::exportable("openvpn"));
    EXPECT_FALSE(schema::exportable("pptp"));
}

TEST(VpnSchema, RequiredFieldsAreChecked)
{
    Strings data, secrets;
    std::string error;
    EXPECT_FALSE(schema::apply("openvpn", {}, {}, {}, false, data, secrets, &error));
    EXPECT_NE(error.find("server"), std::string::npos);
    EXPECT_FALSE(schema::apply("wireguard", {}, {}, {}, false, data, secrets, &error));
    EXPECT_FALSE(schema::apply("openvpn", {{"remote", "vpn.example.com"}, {"port", "70000"}}, {}, {}, false, data,
                               secrets, &error));
    EXPECT_NE(error.find("Port"), std::string::npos);
}

TEST(VpnSchema, OpenVpnRoundTrip)
{
    Strings data, secrets;
    std::string error;
    const schema::Values values{
        {"remote", " vpn.example.com:1194 "},
        {"connection-type", "password-tls"},
        {"username", "alice"},
        {"ca", "/etc/ca.pem"},
        {"cert", "/home/a/cert.pem"},
        {"key", "/home/a/key.pem"},
        {"proto-tcp", "true"},
        {"port", "443"},
        {"compress", "lz4"},
        {"dev-type", ""},
        // Hidden while the proxy is off: must not be written.
        {"proxy-server", "proxy.example.com"},
    };
    ASSERT_TRUE(schema::apply("openvpn", values, {{"cert-pass", SecretNotRequired}},
                              {{"password", "s3cret"}, {"cert-pass", "ignored"}}, false, data, secrets, &error))
        << error;
    EXPECT_EQ(get(data, "remote"), "vpn.example.com:1194");
    EXPECT_EQ(get(data, "connection-type"), "password-tls");
    EXPECT_EQ(get(data, "proto-tcp"), "yes");
    EXPECT_EQ(get(data, "port"), "443");
    EXPECT_EQ(get(data, "compress"), "lz4");
    EXPECT_FALSE(data.count("dev-type"));
    EXPECT_FALSE(data.count("proxy-server"));
    EXPECT_EQ(get(data, "password-flags"), "1");
    EXPECT_EQ(get(data, "cert-pass-flags"), "4");
    EXPECT_EQ(get(secrets, "password"), "s3cret");
    EXPECT_FALSE(secrets.count("cert-pass"));

    // Reading it back shows the same form.
    EXPECT_TRUE(formItem("openvpn", data, "proto-tcp").checked);
    EXPECT_EQ(formItem("openvpn", data, "compress").value, "lz4");
    EXPECT_EQ(formItem("openvpn", data, "cert-pass").storage, uint32_t(SecretNotRequired));
    EXPECT_EQ(formItem("openvpn", data, "password").storage, uint32_t(SecretAgentOwned));
    EXPECT_EQ(formItem("openvpn", data, "password").value, "");

    // A shared profile's passwords belong to the system; "ask every time" keeps none.
    Strings sharedData, sharedSecrets;
    EXPECT_TRUE(schema::apply("openvpn", values, {{"password", SecretNotSaved}}, {{"password", "typed"}}, true,
                              sharedData, sharedSecrets, &error));
    EXPECT_EQ(get(sharedData, "password-flags"), "2");
    EXPECT_EQ(get(sharedData, "cert-pass-flags"), "0");
    EXPECT_TRUE(sharedSecrets.empty());
}

TEST(VpnSchema, KeysTheFormDoesNotKnowStay)
{
    Strings data{{"remote", "old.example.com"}, {"tls-remote", "legacy"}, {"cipher", "AES-256-GCM"}};
    Strings secrets;
    EXPECT_TRUE(schema::apply("openvpn", {{"remote", "new.example.com"}, {"cipher", ""}}, {}, {}, false, data, secrets,
                              nullptr));
    EXPECT_EQ(get(data, "remote"), "new.example.com");
    EXPECT_EQ(get(data, "tls-remote"), "legacy");
    EXPECT_FALSE(data.count("cipher"));
    // A choice value from another editor is offered as itself.
    const auto choices = formItem("openvpn", {{"auth", "whirlpool"}}, "auth").choices;
    ASSERT_FALSE(choices.empty());
    EXPECT_EQ(choices.back().value, "whirlpool");
}

TEST(VpnSchema, PppOptionsAreInvertedAndExclusive)
{
    Strings data, secrets;
    EXPECT_TRUE(schema::apply("pptp",
                              {{"gateway", "pptp.example.com"},
                               {"refuse-pap", "false"},
                               {"refuse-chap", "true"},
                               {"nobsdcomp", "false"},
                               {"require-mppe", "require-mppe-128"},
                               {"lcp-echo-interval", "true"}},
                              {}, {}, false, data, secrets, nullptr));
    EXPECT_EQ(get(data, "refuse-pap"), "yes");
    EXPECT_FALSE(data.count("refuse-chap"));
    EXPECT_EQ(get(data, "nobsdcomp"), "yes");
    EXPECT_EQ(get(data, "require-mppe-128"), "yes");
    EXPECT_FALSE(data.count("require-mppe"));
    EXPECT_EQ(get(data, "lcp-echo-interval"), "30");
    EXPECT_EQ(get(data, "lcp-echo-failure"), "5");

    EXPECT_FALSE(formItem("pptp", data, "refuse-pap").checked);
    EXPECT_TRUE(formItem("pptp", data, "refuse-chap").checked);
    EXPECT_EQ(formItem("pptp", data, "require-mppe").value, "require-mppe-128");
    EXPECT_TRUE(formItem("pptp", data, "lcp-echo-interval").checked);

    // Switching encryption to "any" replaces the 128-bit key, and the echo off removes both keys.
    EXPECT_TRUE(schema::apply("pptp",
                              {{"gateway", "pptp.example.com"},
                               {"require-mppe", "require-mppe"},
                               {"lcp-echo-interval", "false"}},
                              {}, {}, false, data, secrets, nullptr));
    EXPECT_EQ(get(data, "require-mppe"), "yes");
    EXPECT_FALSE(data.count("require-mppe-128"));
    EXPECT_FALSE(data.count("lcp-echo-interval"));
    EXPECT_FALSE(data.count("lcp-echo-failure"));
}

TEST(VpnSchema, SstpDefaultsAreYes)
{
    // tls-ext and refuse-pap default to yes: only turning them off writes a key.
    EXPECT_TRUE(formItem("sstp", {}, "tls-ext").checked);
    EXPECT_FALSE(formItem("sstp", {}, "refuse-pap").checked);
    Strings data, secrets;
    EXPECT_TRUE(schema::apply("sstp",
                              {{"gateway", "sstp.example.com"},
                               {"tls-ext", "false"},
                               {"refuse-pap", "true"},
                               {"refuse-chap", "false"}},
                              {}, {}, false, data, secrets, nullptr));
    EXPECT_EQ(get(data, "tls-ext"), "no");
    EXPECT_FALSE(data.count("refuse-pap"));
    EXPECT_EQ(get(data, "refuse-chap"), "yes");
    EXPECT_FALSE(formItem("sstp", data, "tls-ext").checked);
}

TEST(VpnSchema, LegacyTypeKeysFollowTheFlags)
{
    Strings data, secrets;
    EXPECT_TRUE(schema::apply("vpnc",
                              {{"IPSec gateway", "vpn.example.com"}, {"IPSec ID", "group"}, {"IKE Authmode", "true"}},
                              {{"Xauth password", SecretNotSaved}, {"IPSec secret", SecretNotRequired}},
                              {{"Xauth password", "x"}}, false, data, secrets, nullptr));
    EXPECT_EQ(get(data, "xauth-password-type"), "ask");
    EXPECT_EQ(get(data, "ipsec-secret-type"), "unused");
    EXPECT_EQ(get(data, "IKE Authmode"), "hybrid");
    EXPECT_TRUE(secrets.empty());
    EXPECT_TRUE(schema::apply("libreswan", {{"right", "vpn.example.com"}, {"rekey", "true"}},
                              {{"pskvalue", SecretAgentOwned}}, {{"pskvalue", "group"}}, false, data, secrets, nullptr));
    EXPECT_EQ(get(data, "pskinputmodes"), "save");
    EXPECT_EQ(get(data, "rekey"), "no");
    EXPECT_EQ(get(secrets, "pskvalue"), "group");
}

TEST(VpnSchema, FortiOtpFlagsAreNumbers)
{
    Strings data, secrets;
    EXPECT_TRUE(schema::apply("fortisslvpn",
                              {{"gateway", "f.example.com"}, {"otp-flags", "true"}, {"2fa-flags", "true"}}, {}, {},
                              false, data, secrets, nullptr));
    EXPECT_EQ(get(data, "otp-flags"), "2");
    EXPECT_EQ(get(data, "2fa-flags"), "1");
    EXPECT_TRUE(formItem("fortisslvpn", data, "otp-flags").checked);
    EXPECT_TRUE(schema::apply("fortisslvpn", {{"gateway", "f.example.com"}, {"otp-flags", "false"}}, {}, {}, false,
                              data, secrets, nullptr));
    // Off is the absent key, which the plugin reads as flags 0.
    EXPECT_FALSE(data.count("otp-flags"));
    EXPECT_FALSE(data.count("2fa-flags"));
}

TEST(VpnSchema, ConditionsAcceptTheEmptyValue)
{
    const std::vector<schema::Field> fields = schema::fields("l2tp");
    schema::Field user;
    for (const schema::Field& field : fields) {
        if (field.key == "user")
            user = field;
    }
    EXPECT_TRUE(schema::shown(user, fields, {{"user-auth-type", ""}}));
    EXPECT_FALSE(schema::shown(user, fields, {{"user-auth-type", "tls"}}));
}
