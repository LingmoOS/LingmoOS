// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <unistd.h>

#include "lingmo/networkmanager/variants.h"
#include "lingmo/networkmanager/vpn_utils.h"

using namespace Lingmo::Networkmanager;
using namespace Lingmo::Networkmanager::variants;
using Lingmo::Base::dbus::VariantMap;

static const std::string openvpn = "org.freedesktop.NetworkManager.openvpn";

namespace {

class VpnUtilsTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        char pattern[] = "/tmp/vpnutils-test-XXXXXX";
        directory = mkdtemp(pattern);
        std::ofstream name(directory + "/nm-openvpn-service.name");
        name << "[VPN Connection]\nname=openvpn\nservice=org.freedesktop.NetworkManager.openvpn\n"
                "program=/usr/libexec/nm-openvpn-service\n\n[libnm]\nplugin=libnm-vpn-plugin-openvpn.so\n";
        name.close();
        vpn::setPluginDirectories({directory});
    }

    static void TearDownTestSuite()
    {
        std::filesystem::remove_all(directory);
        vpn::setPluginDirectories({"/etc/NetworkManager/VPN", "/usr/lib/NetworkManager/VPN"});
    }

    static std::string directory;
};

std::string VpnUtilsTest::directory;

} // namespace

TEST_F(VpnUtilsTest, ReadsInstalledPlugins)
{
    EXPECT_EQ(vpn::installedPlugins(), std::vector<std::string>{"openvpn"});
    EXPECT_TRUE(vpn::pluginInstalled(openvpn));
    EXPECT_FALSE(vpn::pluginInstalled("org.freedesktop.NetworkManager.l2tp"));
    EXPECT_EQ(vpn::package("l2tp"), "network-manager-l2tp");
}

TEST_F(VpnUtilsTest, UserNames)
{
    EXPECT_EQ(vpn::userNameKey(openvpn, {{"connection-type", "password"}}), "username");
    EXPECT_EQ(vpn::userNameKey(openvpn, {{"connection-type", "tls"}}), "");
    EXPECT_EQ(vpn::userNameKey("org.freedesktop.NetworkManager.l2tp", {}), "user");
    EXPECT_EQ(vpn::userNameKey("org.freedesktop.NetworkManager.vpnc", {}), "Xauth username");
}

TEST_F(VpnUtilsTest, KindsByHand)
{
    const std::vector<vpn::Kind> kinds = vpn::manualKinds();
    ASSERT_EQ(kinds.size(), 12u);
    EXPECT_EQ(kinds[0].kind, "wireguard");
    EXPECT_TRUE(kinds[0].installed);
    EXPECT_EQ(kinds[1].kind, "openvpn");
    EXPECT_TRUE(kinds[1].installed);
    for (const vpn::Kind& kind : kinds) {
        if (kind.kind == "l2tp")
            EXPECT_FALSE(kind.installed);
    }
}

TEST_F(VpnUtilsTest, WireGuardByHand)
{
    std::string error;
    vpn::WireGuardForm form;
    form.name = "Home";
    form.privateKey = "yAnz5TF+lXXJte14tji3zlMNq+hd2rYUIgJBgB3fBmk=";
    form.addresses = "10.0.0.2/24, fd00::2";
    form.dns = "10.0.0.1, corp.example";
    form.mtu = "1420";
    form.peerRoutes = false;
    form.peers = {{.publicKey = "xTIBA5rboUvnH4htodjb6e697QjLERt1NAB4mZqp8Dg=",
                   .endpoint = "2001:db8::1",
                   .keepalive = "25"}};
    const auto profile = vpn::wireGuardSettings(form, {}, &error);
    ASSERT_TRUE(profile) << error;
    EXPECT_EQ(toString(value(profile->at("connection"), "id")), "Home");
    EXPECT_EQ(toString(value(profile->at("connection"), "interface-name")), "Home");
    EXPECT_EQ(toUInt(value(profile->at("wireguard"), "mtu")), 1420u);
    EXPECT_FALSE(toBool(value(profile->at("wireguard"), "peer-routes")));
    const std::vector<VariantMap> peers = toMapList(value(profile->at("wireguard"), "peers"));
    ASSERT_EQ(peers.size(), 1u);
    EXPECT_EQ(toString(value(peers[0], "endpoint")), "[2001:db8::1]:51820");
    EXPECT_EQ(toStringList(value(peers[0], "allowed-ips")), (std::vector<std::string>{"0.0.0.0/0", "::/0"}));
    EXPECT_EQ(toUInt(value(peers[0], "persistent-keepalive")), 25u);
    EXPECT_EQ(toString(value(profile->at("ipv6"), "method")), "manual");
    EXPECT_EQ(toStringList(value(profile->at("ipv4"), "dns-search")), std::vector<std::string>{"corp.example"});

    // The form reads back as it was filled, minus the secrets.
    const vpn::WireGuardForm back = vpn::wireGuardForm(*profile);
    EXPECT_EQ(back.addresses, "10.0.0.2/24, fd00::2/128");
    EXPECT_EQ(back.dns, "10.0.0.1, corp.example");
    EXPECT_EQ(back.privateKey, "");
    EXPECT_EQ(back.mtu, "1420");
    ASSERT_EQ(back.peers.size(), 1u);
    EXPECT_EQ(back.peers[0].endpoint, "[2001:db8::1]:51820");
    EXPECT_FALSE(back.peers[0].hasPresharedKey);

    // Edited with the keys left empty, the stored ones stay; the written file
    // is one the reader takes back.
    const Settings stored = *profile;
    vpn::WireGuardForm editForm = back;
    editForm.name = "Home 2";
    editForm.addresses = "10.0.0.3/24";
    const auto edited = vpn::wireGuardSettings(editForm, stored, &error);
    ASSERT_TRUE(edited) << error;
    EXPECT_EQ(value(edited->at("connection"), "uuid"), value(profile->at("connection"), "uuid"));
    EXPECT_EQ(toString(value(edited->at("connection"), "id")), "Home 2");
    EXPECT_EQ(toString(value(edited->at("wireguard"), "private-key")), "yAnz5TF+lXXJte14tji3zlMNq+hd2rYUIgJBgB3fBmk=");
    EXPECT_EQ(toString(value(edited->at("ipv6"), "method")), "disabled");

    const std::string config = vpn::wireGuardConfig(*profile);
    EXPECT_NE(config.find("Table = off\n"), std::string::npos);
    EXPECT_NE(config.find("PersistentKeepalive = 25\n"), std::string::npos);
    const auto reread = vpn::wireGuardProfile(config, "Home", &error);
    ASSERT_TRUE(reread) << error;
    EXPECT_EQ(value(reread->at("wireguard"), "peers"), value(profile->at("wireguard"), "peers"));
    EXPECT_EQ(reread->at("ipv4"), profile->at("ipv4"));

    EXPECT_FALSE(vpn::wireGuardSettings({.privateKey = "short"}, {}, &error));
    EXPECT_FALSE(error.empty());
    vpn::WireGuardForm noPeer = form;
    noPeer.peers.clear();
    EXPECT_FALSE(vpn::wireGuardSettings(noPeer, {}, &error));
    vpn::WireGuardForm badAddress = form;
    badAddress.addresses = "10.0.0.2/40";
    EXPECT_FALSE(vpn::wireGuardSettings(badAddress, {}, &error));
}

TEST_F(VpnUtilsTest, WireGuardSearchDomainsSurviveSaves)
{
    std::string error;
    vpn::WireGuardForm form;
    form.privateKey = "yAnz5TF+lXXJte14tji3zlMNq+hd2rYUIgJBgB3fBmk=";
    form.addresses = "10.0.0.2/24, fd00::2/64";
    form.dns = "10.0.0.1, fd00::1, corp.example";
    form.peers = {{.publicKey = "xTIBA5rboUvnH4htodjb6e697QjLERt1NAB4mZqp8Dg="}};
    auto profile = vpn::wireGuardSettings(form, {}, &error);
    ASSERT_TRUE(profile) << error;
    EXPECT_EQ(toStringList(value(profile->at("ipv6"), "dns-search")), std::vector<std::string>{"corp.example"});

    for (int save = 0; save < 2; ++save) {
        const vpn::WireGuardForm back = vpn::wireGuardForm(*profile);
        EXPECT_EQ(back.dns, "10.0.0.1, corp.example, fd00::1");
        profile = vpn::wireGuardSettings(back, *profile, &error);
        ASSERT_TRUE(profile) << error;
        EXPECT_EQ(toStringList(value(profile->at("ipv4"), "dns-search")), std::vector<std::string>{"corp.example"});
        EXPECT_EQ(toStringList(value(profile->at("ipv6"), "dns-search")), std::vector<std::string>{"corp.example"});
    }
}

TEST_F(VpnUtilsTest, Failures)
{
    // NM_ACTIVE_CONNECTION_STATE_REASON_USER_DISCONNECTED and _NO_SECRETS say nothing.
    EXPECT_EQ(vpn::failureMessage(2), "");
    EXPECT_EQ(vpn::failureMessage(9), "");
    EXPECT_NE(vpn::failureMessage(10), "");
}

TEST_F(VpnUtilsTest, ReadsWireGuard)
{
    const std::string config = "[Interface]\n"
                               "PrivateKey = yAnz5TF+lXXJte14tji3zlMNq+hd2rYUIgJBgB3fBmk=  # mine\n"
                               "Address = 10.99.0.2/24, fd00::2\n"
                               "DNS = 10.99.0.1, corp.example\n"
                               "\n"
                               "[Peer]\n"
                               "PublicKey = xTIBA5rboUvnH4htodjb6e697QjLERt1NAB4mZqp8Dg=\n"
                               "AllowedIPs = 10.99.0.0/24, 10.98.0.0/24\n"
                               "Endpoint = vpn.example.com:51820\n"
                               "PersistentKeepalive = 25\n";
    EXPECT_TRUE(vpn::isWireGuardConfig(config));

    std::string error;
    const auto profile = vpn::wireGuardProfile(config, "Cafe (Tokyo)", &error);
    ASSERT_TRUE(profile) << error;
    EXPECT_EQ(toString(value(profile->at("connection"), "id")), "Cafe (Tokyo)");
    EXPECT_EQ(toString(value(profile->at("connection"), "interface-name")), "CafeTokyo");

    const std::vector<VariantMap> peers = toMapList(value(profile->at("wireguard"), "peers"));
    ASSERT_EQ(peers.size(), 1u);
    EXPECT_EQ(toStringList(value(peers[0], "allowed-ips")), (std::vector<std::string>{"10.99.0.0/24", "10.98.0.0/24"}));
    EXPECT_EQ(value(peers[0], "persistent-keepalive").signature(), "u");

    const VariantMap& ipv4 = profile->at("ipv4");
    EXPECT_EQ(toString(value(ipv4, "method")), "manual");
    EXPECT_EQ(toUInt(value(toMapList(value(ipv4, "address-data")).front(), "prefix")), 24u);
    EXPECT_EQ(toStringList(value(ipv4, "dns-search")), std::vector<std::string>{"corp.example"});
    EXPECT_EQ(toUInt(value(toMapList(value(profile->at("ipv6"), "address-data")).front(), "prefix")), 128u);
}

TEST_F(VpnUtilsTest, RejectsWhatIsNotWireGuard)
{
    std::string error;
    EXPECT_FALSE(vpn::wireGuardProfile("[Interface]\nPrivateKey = short\n", "x", &error));
    EXPECT_FALSE(error.empty());
    EXPECT_FALSE(vpn::wireGuardProfile(
        "[Interface]\nPrivateKey = yAnz5TF+lXXJte14tji3zlMNq+hd2rYUIgJBgB3fBmk=\nFoo = 1\n", "x", &error));
    EXPECT_FALSE(vpn::isWireGuardConfig("client\nremote vpn.example.com 1194\n"));
}
