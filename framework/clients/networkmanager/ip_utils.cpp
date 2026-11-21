// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/networkmanager/ip_utils.h"
#include "lingmo/networkmanager/client.h"
#include "lingmo/networkmanager/inet.h"
#include "lingmo/networkmanager/text.h"
#include "lingmo/networkmanager/variants.h"
#include "lingmo/networkmanager/vpn_utils.h"
#include "framework_i18n.h"

#include <algorithm>
#include <bit>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <endian.h>

namespace Lingmo::Networkmanager::ip {

using base::dbus::Variant;
using base::dbus::VariantMap;
using namespace variants;
using Lingmo::framework::tr;
using Lingmo::i18n::format;

static const std::string ipv4Setting = "ipv4";
static const std::string ipv6Setting = "ipv6";

static const VariantMap& group(const Settings& settings, const std::string& name)
{
    static const VariantMap empty;
    const auto found = settings.find(name);
    return found == settings.end() ? empty : found->second;
}

static std::string methodOf(const Settings& settings, const std::string& family)
{
    // A profile without the setting gets "auto" from NetworkManager's normalization.
    const std::string method = toString(value(group(settings, family), "method"));
    return method.empty() ? "auto" : method;
}

static std::string methodLabel(const std::string& method, bool v4)
{
    if (method == "auto")
        return v4 ? tr("Automatic (DHCP)") : tr("Automatic");
    if (method == "manual")
        return tr("Manual");
    if (method == "disabled")
        return tr("Off");
    if (method == "dhcp")
        return tr("DHCP Only");
    if (method == "link-local")
        return tr("Link-Local Only");
    if (method == "shared")
        return tr("Shared to Other Computers");
    if (method == "ignore")
        return tr("Left to the System");
    return method;
}

static std::vector<Method> methods(const std::string& current, bool v4)
{
    std::vector<std::string> values{"auto", "manual", "disabled"};
    if (std::find(values.begin(), values.end(), current) == values.end())
        values.push_back(current);
    std::vector<Method> list;
    for (const std::string& value : values)
        list.push_back({value, methodLabel(value, v4)});
    return list;
}

bool configurable(const std::string& method)
{
    // Whether the family takes addresses, gateways and name servers in
    // NetworkManager's verify(); "shared" takes addresses but its name
    // servers are the shared ones.
    return method == "auto" || method == "manual" || method == "dhcp";
}

static bool hostAddress(const std::string& value, bool v4)
{
    if (v4) {
        const auto address = inet::parseIpv4(value);
        return address && *address != 0 && *address != 0xffffffffu;
    }
    // A scope such as "%eth0" would be a link-local address on one interface.
    const auto address = inet::parseIpv6(value);
    if (!address)
        return false;
    for (const uint8_t byte : *address) {
        if (byte)
            return true;
    }
    return false;
}

// "255.255.255.0", or a prefix length, with or without its slash.
static int parsePrefix(const std::string& value, bool v4)
{
    std::string prefix = value;
    if (!prefix.empty() && prefix[0] == '/')
        prefix.erase(0, 1);
    if (!prefix.empty()) {
        char* end = nullptr;
        const long length = std::strtol(prefix.c_str(), &end, 10);
        if (end && *end == '\0')
            return length >= 1 && length <= (v4 ? 32 : 128) ? int(length) : -1;
    }

    const auto mask = v4 ? inet::parseIpv4(value) : std::nullopt;
    if (!mask || *mask == 0)
        return -1;
    // The ones must come first: 255.0.255.0 is no netmask.
    const uint32_t hostBits = ~*mask;
    if (hostBits & (hostBits + 1))
        return -1;
    return std::popcount(*mask);
}

static std::string netmask(uint32_t prefix)
{
    if (prefix == 0 || prefix > 32)
        return {};
    return inet::formatIpv4(uint32_t(0xffffffffu << (32 - prefix)));
}

static std::vector<std::string> dnsWords(const std::string& value)
{
    std::vector<std::string> words;
    std::string word;
    for (const char c : value) {
        if (c == ',' || std::isspace(static_cast<unsigned char>(c))) {
            if (!word.empty())
                words.push_back(std::move(word));
            word.clear();
        } else {
            word += c;
        }
    }
    if (!word.empty())
        words.push_back(word);
    return words;
}

// A name server with the server name DNS over TLS checks, as dns-data writes it.
static std::string dnsAddress(const std::string& word)
{
    return word.substr(0, word.find('#'));
}

static std::vector<std::string> dnsServers(const Settings& settings)
{
    std::vector<std::string> servers;
    for (const std::string& family : {ipv4Setting, ipv6Setting}) {
        const VariantMap& ip = group(settings, family);
        // NetworkManager 1.56 sends both; its daemon takes "dns" when a client sends both.
        if (contains(ip, "dns-data")) {
            for (const std::string& server : toStringList(value(ip, "dns-data")))
                servers.push_back(server);
        } else if (family == ipv4Setting) {
            // Network byte order.
            for (const uint32_t server : toUIntList(value(ip, "dns")))
                servers.push_back(inet::formatIpv4(be32toh(server)));
        } else {
            for (const std::string& server : toBytesList(value(ip, "dns"))) {
                const std::string address = inet::formatIpv6Bytes(server);
                if (!address.empty())
                    servers.push_back(address);
            }
        }
    }
    return servers;
}

Form form(const Settings& settings)
{
    Form form;
    for (const bool v4 : {true, false}) {
        const std::string& family = v4 ? ipv4Setting : ipv6Setting;
        const VariantMap& ip = group(settings, family);
        const std::string method = methodOf(settings, family);
        const std::vector<VariantMap> addresses = toMapList(value(ip, "address-data"));
        const VariantMap first = addresses.empty() ? VariantMap() : addresses.front();
        const uint32_t length = toUInt(value(first, "prefix"));

        if (v4) {
            form.ipv4Method = method;
            form.ipv4Methods = methods(method, true);
            form.ipv4Address = toString(value(first, "address"));
            form.ipv4Netmask = netmask(length);
            form.ipv4Gateway = toString(value(ip, "gateway"));
        } else {
            form.ipv6Method = method;
            form.ipv6Methods = methods(method, false);
            form.ipv6Address = toString(value(first, "address"));
            form.ipv6Prefix = length ? std::to_string(length) : std::string();
            form.ipv6Gateway = toString(value(ip, "gateway"));
        }
    }

    // The switch stands for both families; IPv4's decides when they differ,
    // unless IPv4 takes no name servers and so keeps a value nothing writes.
    const std::string& dnsFamily =
        configurable(form.ipv4Method) || !configurable(form.ipv6Method) ? ipv4Setting : ipv6Setting;
    form.automaticDns = !toBool(value(group(settings, dnsFamily), "ignore-auto-dns"));
    form.dns = text::join(dnsServers(settings), ", ");
    return form;
}

Form currentAddresses(const Client& client, const std::string& connection)
{
    // Read off the device, as the details rows are.
    Form current;
    const std::string active = vpn::activeConnection(client, connection);
    const std::vector<std::string> devices = toStringList(client.property(active, activeInterface, "Devices"));
    if (devices.empty())
        return current;
    const std::string& device = devices.front();

    const std::string ip4 = objectPath(client.property(device, deviceInterface, "Ip4Config"));
    const std::vector<VariantMap> addresses4 = toMapList(client.property(ip4, ip4ConfigInterface, "AddressData"));
    if (!addresses4.empty()) {
        current.ipv4Address = toString(value(addresses4.front(), "address"));
        current.ipv4Netmask = netmask(toUInt(value(addresses4.front(), "prefix")));
        current.ipv4Gateway = toString(client.property(ip4, ip4ConfigInterface, "Gateway"));
    }

    // The global address, not the link-local one every interface has.
    const std::string ip6 = objectPath(client.property(device, deviceInterface, "Ip6Config"));
    for (const VariantMap& address : toMapList(client.property(ip6, ip6ConfigInterface, "AddressData"))) {
        const std::string host = toString(value(address, "address"));
        if (inet::isLinkLocal(host))
            continue;
        current.ipv6Address = inet::normalized(host);
        current.ipv6Prefix = std::to_string(toUInt(value(address, "prefix")));
        current.ipv6Gateway = toString(client.property(ip6, ip6ConfigInterface, "Gateway"));
        break;
    }
    return current;
}

Check check(const Form& form)
{
    bool complete = true;
    std::string message;
    const auto fail = [&message](const std::string& text) {
        if (message.empty())
            message = text;
    };

    for (const bool v4 : {true, false}) {
        if (text::trimmed(v4 ? form.ipv4Method : form.ipv6Method) != "manual")
            continue;

        const std::string address = text::trimmed(v4 ? form.ipv4Address : form.ipv6Address);
        const std::string length = text::trimmed(v4 ? form.ipv4Netmask : form.ipv6Prefix);
        const std::string gateway = text::trimmed(v4 ? form.ipv4Gateway : form.ipv6Gateway);
        if (address.empty())
            complete = false;
        else if (!hostAddress(address, v4))
            fail(format(v4 ? tr("{address} isn't an IPv4 address.") : tr("{address} isn't an IPv6 address."),
                        {{"address", address}}));
        // IPv6 has a usual prefix length; an IPv4 network has none to assume.
        if (length.empty() && v4)
            complete = false;
        else if (!length.empty() && parsePrefix(length, v4) < 0)
            fail(v4 ? format(tr("{mask} isn't a subnet mask."), {{"mask", length}})
                    : std::string(tr("The prefix length must be a number from 1 to 128.")));
        if (!gateway.empty() && !hostAddress(gateway, v4))
            fail(format(tr("{address} isn't a router address."), {{"address", gateway}}));
    }

    const bool ipv4Dns = configurable(text::trimmed(form.ipv4Method));
    const bool ipv6Dns = configurable(text::trimmed(form.ipv6Method));
    for (const std::string& word : dnsWords(form.dns)) {
        const std::string address = dnsAddress(word);
        if (hostAddress(address, true)) {
            if (!ipv4Dns)
                fail(tr("IPv4 name servers need IPv4 set to automatic or manual."));
        } else if (hostAddress(address, false)) {
            if (!ipv6Dns)
                fail(tr("IPv6 name servers need IPv6 set to automatic or manual."));
        } else {
            fail(format(tr("{address} isn't a name server address."), {{"address", word}}));
        }
    }

    return {complete && message.empty(), message};
}

std::optional<Settings> settings(const Form& form, const Settings& existing, bool* changed, std::string* error)
{
    const Check result = check(form);
    if (!result.complete) {
        if (error)
            *error = result.message.empty() ? std::string(tr("Fill in the address and subnet mask.")) : result.message;
        return std::nullopt;
    }

    // Compared as the form shows them; the address fields count only for a
    // manual setup, as they are hidden otherwise.
    const Form stored = ip::form(existing);
    const auto differs = [&form, &stored](bool v4) {
        const std::string method = text::trimmed(v4 ? form.ipv4Method : form.ipv6Method);
        if (method != text::trimmed(v4 ? stored.ipv4Method : stored.ipv6Method))
            return true;
        if (method != "manual")
            return false;
        if (v4) {
            return text::trimmed(form.ipv4Address) != text::trimmed(stored.ipv4Address)
                   || text::trimmed(form.ipv4Netmask) != text::trimmed(stored.ipv4Netmask)
                   || text::trimmed(form.ipv4Gateway) != text::trimmed(stored.ipv4Gateway);
        }
        return text::trimmed(form.ipv6Address) != text::trimmed(stored.ipv6Address)
               || text::trimmed(form.ipv6Prefix) != text::trimmed(stored.ipv6Prefix)
               || text::trimmed(form.ipv6Gateway) != text::trimmed(stored.ipv6Gateway);
    };
    const bool ipv4Changed = differs(true);
    const bool ipv6Changed = differs(false);
    // The servers as a list, so "1.1.1.1,8.8.8.8" is the same as the stored "1.1.1.1, 8.8.8.8".
    const bool dnsChanged = form.automaticDns != stored.automaticDns || dnsWords(form.dns) != dnsWords(stored.dns);
    if (changed)
        *changed = ipv4Changed || ipv6Changed || dnsChanged;

    Settings profile = existing;
    for (const bool v4 : {true, false}) {
        const std::string& family = v4 ? ipv4Setting : ipv6Setting;
        if (!(v4 ? ipv4Changed : ipv6Changed))
            continue;

        VariantMap ip = profile[family];
        const std::string method = text::trimmed(v4 ? form.ipv4Method : form.ipv6Method);
        ip["method"] = Variant(method);
        // NetworkManager's daemon prefers the deprecated "addresses" to
        // "address-data" when both are sent, so it goes.
        ip.erase("addresses");
        ip.erase("gateway");

        if (method == "manual") {
            const std::string length = text::trimmed(v4 ? form.ipv4Netmask : form.ipv6Prefix);
            std::vector<VariantMap> addresses = toMapList(value(ip, "address-data"));
            const VariantMap address{
                {"address", Variant(inet::normalized(text::trimmed(v4 ? form.ipv4Address : form.ipv6Address)))},
                {"prefix", Variant(uint32_t(length.empty() ? 64 : parsePrefix(length, v4)))},
            };
            // Only the first address is in the form; any further ones stay.
            if (addresses.empty())
                addresses.push_back(address);
            else
                addresses.front() = address;
            ip["address-data"] = Variant(addresses);
            const std::string gateway = text::trimmed(v4 ? form.ipv4Gateway : form.ipv6Gateway);
            if (!gateway.empty())
                ip["gateway"] = Variant(inet::normalized(gateway));
        } else if (method != "shared") {
            // As in GNOME's editor: addresses of their own only for a manual setup.
            ip.erase("address-data");
        }

        if (!configurable(method)) {
            for (const char* key : {"dns", "dns-data", "dns-search"})
                ip.erase(key);
        }
        profile[family] = ip;
    }

    if (dnsChanged) {
        std::vector<uint32_t> dns4;
        std::vector<std::vector<uint8_t>> dns6;
        std::vector<std::string> data4;
        std::vector<std::string> data6;
        for (const std::string& word : dnsWords(form.dns)) {
            const std::string address = dnsAddress(word);
            if (const auto v4 = inet::parseIpv4(address)) {
                dns4.push_back(htobe32(*v4));
                data4.push_back(word);
            } else if (const auto v6 = inet::parseIpv6(address)) {
                dns6.emplace_back(v6->begin(), v6->end());
                data6.push_back(word);
            }
        }

        for (const bool v4 : {true, false}) {
            const std::string& family = v4 ? ipv4Setting : ipv6Setting;
            VariantMap ip = profile[family];
            // dns-data only goes to a NetworkManager that sent it: an older
            // one rejects a key it doesn't know.
            const bool data = contains(ip, "dns-data");
            ip.erase("dns");
            ip.erase("dns-data");
            if (data && !(v4 ? data4 : data6).empty())
                ip["dns-data"] = Variant(v4 ? data4 : data6);
            else if (v4 && !dns4.empty())
                ip["dns"] = Variant(dns4);
            else if (!v4 && !dns6.empty())
                ip["dns"] = Variant(dns6);
            // Off, only the servers above are used; on, they come before the network's own.
            if (configurable(methodOf(profile, family)))
                ip["ignore-auto-dns"] = Variant(!form.automaticDns);
            profile[family] = ip;
        }
    }

    return profile;
}

} // namespace Lingmo::Networkmanager::ip
