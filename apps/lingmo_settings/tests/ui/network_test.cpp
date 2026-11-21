// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <cstdio>
#include <string>

#include "pagetest.h"
#include "ui/window.h"
#include "window/options.h"

using namespace Lingmo::ui;
using namespace settings;

namespace {

std::string dumpLine(const std::string& dump, const std::string& name)
{
    const size_t at = dump.find("\"" + name + "\"");
    if (at == std::string::npos)
        return {};
    const size_t start = dump.rfind('\n', at) + 1;
    return dump.substr(start, dump.find('\n', at) - start);
}

bool shows(const PageFixture& page, const std::string& name)
{
    const std::string line = dumpLine(page.dump(), name);
    return !line.empty() && !line.ends_with(" hidden");
}

// The view's top in window coordinates, logical pixels.
double top(const PageFixture& page, const std::string& name)
{
    const std::string line = dumpLine(page.dump(), name);
    const size_t device = line.find(" device ");
    if (device == std::string::npos)
        return -1;
    long x, y, width, height;
    sscanf(line.c_str() + device, " device %ld %ld %ld %ld", &x, &y, &width, &height);
    return y / page.window->window().scale();
}

// The sample networks: the pages show without NetworkManager's state.
struct MockNetwork : testing::Test
{
    void SetUp() override { setOptions({.mockNetwork = true}); }
    void TearDown() override { setOptions({}); }
};

} // namespace

TEST_F(MockNetwork, WifiSplitsTheNetworks)
{
    PageFixture page("wlan");
    EXPECT_TRUE(shows(page, "wlan:switch"));
    // The network in use shares the switch's card.
    EXPECT_TRUE(shows(page, "wifi:Lingmo Studio"));
    EXPECT_LT(top(page, "wifi:Lingmo Studio"), top(page, "wlan:known"));
    // A saved network is known; the others follow, with the way to add a hidden one last.
    EXPECT_TRUE(shows(page, "wlan:known"));
    EXPECT_GT(top(page, "wifi:Coffee Shop"), top(page, "wlan:known"));
    EXPECT_LT(top(page, "wifi:Coffee Shop"), top(page, "wlan:other"));
    EXPECT_GT(top(page, "wifi:Guest Network"), top(page, "wlan:other"));
    EXPECT_GT(top(page, "wifi:Neighbor Wi-Fi"), top(page, "wifi:Guest Network"));
    EXPECT_GT(top(page, "wlan:addOther"), top(page, "wifi:Neighbor Wi-Fi"));
    EXPECT_FALSE(shows(page, "wlan:hotspot"));
}

TEST_F(MockNetwork, EthernetListsTheWiredConnections)
{
    PageFixture page("ethernet");
    EXPECT_TRUE(shows(page, "ethernet:switch"));
    EXPECT_TRUE(shows(page, "wired:Wired connection 1"));
    EXPECT_TRUE(shows(page, "wired:USB Ethernet"));
    // The connected one comes first.
    EXPECT_LT(top(page, "wired:Wired connection 1"), top(page, "wired:USB Ethernet"));
}

TEST(NetworkPage, LeadsToVpnAndProxy)
{
    PageFixture page("network");
    EXPECT_TRUE(shows(page, "network:vpn"));
    EXPECT_TRUE(shows(page, "network:proxy"));
    EXPECT_LT(top(page, "network:vpn"), top(page, "network:proxy"));
}

TEST(VpnPage, OffersAddingAndImporting)
{
    PageFixture page("network", "vpn");
    EXPECT_TRUE(shows(page, "vpn:add"));
    EXPECT_TRUE(shows(page, "vpn:import"));
    EXPECT_TRUE(shows(page, "vpn:note"));
    EXPECT_LT(top(page, "vpn:add"), top(page, "vpn:import"));
    EXPECT_LT(top(page, "vpn:import"), top(page, "vpn:note"));
}

TEST(HotspotPage, ShowsTheNameAndPassword)
{
    PageFixture page("network", "hotspot");
    EXPECT_TRUE(shows(page, "hotspot:switch"));
    EXPECT_TRUE(shows(page, "hotspot:status"));
    EXPECT_TRUE(shows(page, "hotspot:name"));
    EXPECT_TRUE(shows(page, "hotspot:password"));
    // The generated name and password are valid, so nothing is wrong yet.
    EXPECT_FALSE(shows(page, "hotspot:validation"));
}
