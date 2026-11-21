// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <endian.h>
#include <gtest/gtest.h>

#include "lingmo/networkmanager/ip_utils.h"
#include "lingmo/networkmanager/variants.h"

using namespace Lingmo::Networkmanager;
using Lingmo::Base::dbus::Variant;
using Lingmo::Base::dbus::VariantMap;
using namespace Lingmo::Networkmanager::variants;

static Variant addresses(const std::vector<std::pair<std::string, uint32_t>>& entries)
{
    std::vector<VariantMap> list;
    for (const auto& [address, prefix] : entries)
        list.push_back({{"address", Variant(address)}, {"prefix", Variant(prefix)}});
    return Variant(list);
}

static Settings automaticProfile()
{
    return {
        {"connection", {{"id", Variant(std::string("Home"))}}},
        {"ipv4",
         {{"method", Variant(std::string("auto"))},
          {"address-data", addresses({})},
          {"addresses", Variant(std::vector<std::vector<uint32_t>>())}}},
        {"ipv6", {{"method", Variant(std::string("auto"))}}},
    };
}

static std::vector<std::string> methodValues(const std::vector<ip::Method>& methods)
{
    std::vector<std::string> values;
    for (const ip::Method& method : methods)
        values.push_back(method.value);
    return values;
}

TEST(IpUtils, ReadsAnAutomaticProfile)
{
    const ip::Form form = ip::form(automaticProfile());
    EXPECT_EQ(form.ipv4Method, "auto");
    EXPECT_EQ(form.ipv4Address, "");
    EXPECT_TRUE(form.automaticDns);
    EXPECT_EQ(form.dns, "");
    EXPECT_EQ(methodValues(form.ipv4Methods), (std::vector<std::string>{"auto", "manual", "disabled"}));
    EXPECT_EQ(form.ipv4Methods.front().label, "Automatic (DHCP)");
    EXPECT_EQ(form.ipv6Methods.front().label, "Automatic");

    // A profile without an ipv6 setting is "auto" once NetworkManager normalizes it.
    Settings profile = automaticProfile();
    profile.erase("ipv6");
    EXPECT_EQ(ip::form(profile).ipv6Method, "auto");
}

TEST(IpUtils, ReadsAManualProfile)
{
    Settings profile = automaticProfile();
    profile["ipv4"] = {
        {"method", Variant(std::string("manual"))},
        {"address-data", addresses({{"192.168.1.20", 24}, {"10.0.0.2", 8}})},
        {"gateway", Variant(std::string("192.168.1.1"))},
        {"dns", Variant(std::vector<uint32_t>{htobe32(0x01010101u)})},
        {"ignore-auto-dns", Variant(true)},
    };
    std::vector<uint8_t> server(16, 0);
    server[0] = 0x26;
    server[1] = 0x06;
    server[15] = 1;
    profile["ipv6"] = {
        {"method", Variant(std::string("ignore"))},
        {"dns", Variant(std::vector<std::vector<uint8_t>>{server})},
    };

    const ip::Form form = ip::form(profile);
    EXPECT_EQ(form.ipv4Address, "192.168.1.20");
    EXPECT_EQ(form.ipv4Netmask, "255.255.255.0");
    EXPECT_EQ(form.ipv4Gateway, "192.168.1.1");
    EXPECT_FALSE(form.automaticDns);
    EXPECT_EQ(form.dns, "1.1.1.1, 2606::1");
    // A method the form doesn't offer stays a choice while the profile has it.
    EXPECT_EQ(methodValues(form.ipv6Methods), (std::vector<std::string>{"auto", "manual", "disabled", "ignore"}));
    EXPECT_EQ(form.ipv6Methods.back().label, "Left to the System");
}

TEST(IpUtils, PrefersDnsData)
{
    Settings profile = automaticProfile();
    profile["ipv4"]["dns"] = Variant(std::vector<uint32_t>{htobe32(0x01010101u)});
    profile["ipv4"]["dns-data"] = Variant(std::vector<std::string>{"1.1.1.1#one.one.one.one"});
    EXPECT_EQ(ip::form(profile).dns, "1.1.1.1#one.one.one.one");
}

struct CheckCase
{
    const char* name;
    const char* method;
    const char* address;
    const char* netmask;
    const char* gateway;
    const char* dns;
    bool complete;
    bool message;
};

TEST(IpUtils, ChecksTheForm)
{
    const CheckCase cases[] = {
        {"automatic", "auto", "", "", "", "", true, false},
        {"manual, empty", "manual", "", "", "", "", false, false},
        {"manual, no netmask", "manual", "192.168.1.20", "", "", "", false, false},
        {"manual", "manual", "192.168.1.20", "255.255.255.0", "192.168.1.1", "", true, false},
        {"prefix length", "manual", "192.168.1.20", "24", "", "", true, false},
        {"prefix with slash", "manual", "192.168.1.20", "/24", "", "", true, false},
        {"short address", "manual", "192.168.1", "24", "", "", false, true},
        {"octet too big", "manual", "192.168.1.256", "24", "", "", false, true},
        {"no host", "manual", "0.0.0.0", "24", "", "", false, true},
        {"holes in the netmask", "manual", "192.168.1.20", "255.0.255.0", "", "", false, true},
        {"prefix too long", "manual", "192.168.1.20", "33", "", "", false, true},
        {"IPv6 router", "manual", "192.168.1.20", "24", "fe80::1", "", false, true},
        {"servers", "auto", "", "", "", "1.1.1.1, 8.8.8.8 2606:4700::1111", true, false},
        {"server name", "auto", "", "", "", "1.1.1.1#one.one.one.one", true, false},
        {"not a server", "auto", "", "", "", "dns.example", false, true},
        {"IPv4 server, IPv4 off", "disabled", "", "", "", "1.1.1.1", false, true},
        {"IPv6 server, IPv4 off", "disabled", "", "", "", "2606:4700::1111", true, false},
    };
    for (const CheckCase& test : cases) {
        ip::Form form = ip::form(automaticProfile());
        form.ipv4Method = test.method;
        form.ipv4Address = test.address;
        form.ipv4Netmask = test.netmask;
        form.ipv4Gateway = test.gateway;
        form.dns = test.dns;
        const ip::Check result = ip::check(form);
        EXPECT_EQ(result.complete, test.complete) << test.name;
        EXPECT_EQ(!result.message.empty(), test.message) << test.name;
    }
}

TEST(IpUtils, ChecksIpv6)
{
    ip::Form form = ip::form(automaticProfile());
    form.ipv6Method = "manual";
    form.ipv6Address = "2001:db8::20";
    // The usual prefix length when none is given.
    EXPECT_TRUE(ip::check(form).complete);
    form.ipv6Prefix = "129";
    EXPECT_FALSE(ip::check(form).complete);
    form.ipv6Prefix = "64";
    form.ipv6Address = "fe80::1%eth0";
    EXPECT_FALSE(ip::check(form).message.empty());
    form.ipv6Address = "192.168.1.20";
    EXPECT_FALSE(ip::check(form).message.empty());
}

TEST(IpUtils, WritesAManualSetup)
{
    const Settings existing = automaticProfile();
    ip::Form form = ip::form(existing);
    form.ipv4Method = "manual";
    form.ipv4Address = " 192.168.1.20 ";
    form.ipv4Netmask = "255.255.255.0";
    form.ipv4Gateway = "192.168.1.1";

    bool changed = false;
    std::string error;
    const auto profile = ip::settings(form, existing, &changed, &error);
    ASSERT_TRUE(profile) << error;
    EXPECT_TRUE(changed);

    const VariantMap& ipv4 = profile->at("ipv4");
    EXPECT_EQ(toString(value(ipv4, "method")), "manual");
    EXPECT_EQ(toString(value(ipv4, "gateway")), "192.168.1.1");
    // The daemon would take the deprecated key over the new one.
    EXPECT_FALSE(contains(ipv4, "addresses"));
    EXPECT_EQ(value(ipv4, "address-data").signature(), "aa{sv}");
    const std::vector<VariantMap> data = toMapList(value(ipv4, "address-data"));
    ASSERT_EQ(data.size(), 1u);
    EXPECT_EQ(toString(value(data.front(), "address")), "192.168.1.20");
    EXPECT_EQ(value(data.front(), "prefix").signature(), "u");
    EXPECT_EQ(toUInt(value(data.front(), "prefix")), 24u);
    // Only what changed is written.
    EXPECT_EQ(profile->at("ipv6"), existing.at("ipv6"));
    EXPECT_FALSE(contains(ipv4, "ignore-auto-dns"));

    // The form read back from what was written is the same, so saving it again changes nothing.
    const Settings stored = *profile;
    ip::settings(ip::form(stored), stored, &changed, &error);
    EXPECT_FALSE(changed);
}

TEST(IpUtils, KeepsFurtherAddresses)
{
    Settings existing = automaticProfile();
    existing["ipv4"] = {
        {"method", Variant(std::string("manual"))},
        {"address-data", addresses({{"192.168.1.20", 24}, {"10.0.0.2", 8}})},
    };
    ip::Form form = ip::form(existing);
    form.ipv4Address = "192.168.1.30";

    bool changed = false;
    const auto profile = ip::settings(form, existing, &changed, nullptr);
    ASSERT_TRUE(profile);
    const std::vector<VariantMap> data = toMapList(value(profile->at("ipv4"), "address-data"));
    ASSERT_EQ(data.size(), 2u);
    EXPECT_EQ(toString(value(data[0], "address")), "192.168.1.30");
    EXPECT_EQ(toString(value(data[1], "address")), "10.0.0.2");

    // Back to automatic, as GNOME's editor does it: no addresses of its own.
    form.ipv4Method = "auto";
    const auto automatic = ip::settings(form, existing, &changed, nullptr);
    ASSERT_TRUE(automatic);
    EXPECT_FALSE(contains(automatic->at("ipv4"), "address-data"));
    EXPECT_FALSE(contains(automatic->at("ipv4"), "gateway"));
}

TEST(IpUtils, WritesNameServers)
{
    const Settings existing = automaticProfile();
    ip::Form form = ip::form(existing);
    form.automaticDns = false;
    form.dns = "1.1.1.1,8.8.8.8  2606:4700::1111";

    bool changed = false;
    std::string error;
    const auto profile = ip::settings(form, existing, &changed, &error);
    ASSERT_TRUE(profile) << error;
    EXPECT_TRUE(changed);

    const VariantMap& ipv4 = profile->at("ipv4");
    EXPECT_EQ(value(ipv4, "dns").signature(), "au");
    EXPECT_EQ(toUIntList(value(ipv4, "dns")), (std::vector<uint32_t>{htobe32(0x01010101u), htobe32(0x08080808u)}));
    EXPECT_EQ(value(ipv4, "ignore-auto-dns").signature(), "b");
    EXPECT_TRUE(toBool(value(ipv4, "ignore-auto-dns")));

    const VariantMap& ipv6 = profile->at("ipv6");
    EXPECT_EQ(value(ipv6, "dns").signature(), "aay");
    const std::vector<std::string> servers = toBytesList(value(ipv6, "dns"));
    ASSERT_EQ(servers.size(), 1u);
    EXPECT_EQ(servers.front(), std::string("\x26\x06\x47\x00\0\0\0\0\0\0\0\0\0\0\x11\x11", 16));
    EXPECT_TRUE(toBool(value(ipv6, "ignore-auto-dns")));

    // The same servers written differently are no change.
    const Settings stored = *profile;
    form = ip::form(stored);
    EXPECT_EQ(form.dns, "1.1.1.1, 8.8.8.8, 2606:4700::1111");
    form.dns = "1.1.1.1 8.8.8.8, 2606:4700::1111";
    ip::settings(form, stored, &changed, &error);
    EXPECT_FALSE(changed);

    // Emptied, the keys go.
    form.dns = "";
    form.automaticDns = true;
    const auto cleared = ip::settings(form, stored, &changed, &error);
    ASSERT_TRUE(cleared);
    EXPECT_TRUE(changed);
    EXPECT_FALSE(contains(cleared->at("ipv4"), "dns"));
    EXPECT_FALSE(contains(cleared->at("ipv6"), "dns"));
    EXPECT_FALSE(toBool(value(cleared->at("ipv4"), "ignore-auto-dns")));
}

TEST(IpUtils, AutomaticDnsOfAnIpv6OnlyProfile)
{
    Settings existing = automaticProfile();
    existing["ipv4"] = {{"method", Variant(std::string("disabled"))}};
    ip::Form form = ip::form(existing);
    EXPECT_TRUE(form.automaticDns);
    form.automaticDns = false;
    form.dns = "2606:4700::1111";

    bool changed = false;
    std::string error;
    const auto profile = ip::settings(form, existing, &changed, &error);
    ASSERT_TRUE(profile) << error;
    EXPECT_TRUE(toBool(value(profile->at("ipv6"), "ignore-auto-dns")));

    // Read back, the switch is off as saved, and another server keeps it off.
    form = ip::form(*profile);
    EXPECT_FALSE(form.automaticDns);
    form.dns = "2001:4860:4860::8888";
    const auto edited = ip::settings(form, *profile, &changed, &error);
    ASSERT_TRUE(edited) << error;
    EXPECT_TRUE(toBool(value(edited->at("ipv6"), "ignore-auto-dns")));
}

TEST(IpUtils, WritesDnsDataWhereItCameFrom)
{
    Settings existing = automaticProfile();
    existing["ipv4"]["dns"] = Variant(std::vector<uint32_t>{htobe32(0x01010101u)});
    existing["ipv4"]["dns-data"] = Variant(std::vector<std::string>{"1.1.1.1"});
    ip::Form form = ip::form(existing);
    form.dns = "9.9.9.9#dns.quad9.net";

    bool changed = false;
    const auto profile = ip::settings(form, existing, &changed, nullptr);
    ASSERT_TRUE(profile);
    const VariantMap& ipv4 = profile->at("ipv4");
    // Sending both, the daemon would take the old "dns".
    EXPECT_FALSE(contains(ipv4, "dns"));
    EXPECT_EQ(toStringList(value(ipv4, "dns-data")), std::vector<std::string>{"9.9.9.9#dns.quad9.net"});
}

TEST(IpUtils, TurnsAFamilyOff)
{
    Settings existing = automaticProfile();
    existing["ipv6"]["dns-search"] = Variant(std::vector<std::string>{"example.com"});
    ip::Form form = ip::form(existing);
    form.ipv6Method = "disabled";

    bool changed = false;
    std::string error;
    const auto profile = ip::settings(form, existing, &changed, &error);
    ASSERT_TRUE(profile) << error;
    const VariantMap& ipv6 = profile->at("ipv6");
    EXPECT_EQ(toString(value(ipv6, "method")), "disabled");
    // NetworkManager refuses search domains for a family that is off.
    EXPECT_FALSE(contains(ipv6, "dns-search"));
}

TEST(IpUtils, RefusesAnIncompleteForm)
{
    ip::Form form = ip::form(automaticProfile());
    form.ipv4Method = "manual";
    bool changed = true;
    std::string error;
    EXPECT_FALSE(ip::settings(form, automaticProfile(), &changed, &error));
    EXPECT_FALSE(error.empty());
}
