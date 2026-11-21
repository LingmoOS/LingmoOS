// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <functional>
#include <memory>
#include <string>

#include "lingmo/networkmanager/handler.h"
#include "lingmo/networkmanager/vpn_schema.h"
#include "pages/network/vpn/vpneditor.h"
#include "ui/label.h"
#include "ui/test_input.h"
#include "ui/window.h"

using namespace settings::network;
namespace nm = Lingmo::Networkmanager;
namespace schema = Lingmo::Networkmanager::vpn::schema;

static nm::VpnForm openVpnForm(const std::map<std::string, std::string>& data)
{
    nm::VpnForm form;
    form.kind = "openvpn";
    form.label = "OpenVPN";
    form.installed = true;
    form.name = "Office";
    form.fields = schema::formFields("openvpn", data);
    return form;
}

TEST(VpnEditor, PluginFormNeedsItsServer)
{
    VpnEditor editor;
    editor.load(openVpnForm({}));
    EXPECT_FALSE(editor.complete());

    editor.load(openVpnForm({{"remote", "vpn.example.com"}, {"connection-type", "password"}, {"username", "alice"}}));
    EXPECT_TRUE(editor.complete());
    const nm::VpnValues values = editor.collect();
    EXPECT_EQ(values.kind, "openvpn");
    EXPECT_EQ(values.name, "Office");
    EXPECT_EQ(values.values.at("remote"), "vpn.example.com");
    EXPECT_EQ(values.values.at("username"), "alice");
    // Bools go back as text, false while off.
    EXPECT_EQ(values.values.at("proto-tcp"), "false");
    // A password with no flags is this user's, kept in the keyring.
    EXPECT_EQ(values.storage.at("password"), uint32_t(nm::SecretAgentOwned));
}

TEST(VpnEditor, SecretsFillThePasswords)
{
    VpnEditor editor;
    editor.load(openVpnForm({{"remote", "vpn.example.com"}, {"connection-type", "password"}}));
    nm::VpnSecrets secrets;
    secrets.values = {{"password", "s3cret"}, {"unknown", "dropped"}};
    editor.setSecrets(secrets);
    const nm::VpnValues values = editor.collect();
    EXPECT_EQ(values.secrets.at("password"), "s3cret");
    EXPECT_FALSE(values.secrets.count("unknown"));
}

TEST(VpnEditor, MissingPluginShowsNoForm)
{
    nm::VpnForm form;
    form.kind = "l2tp";
    form.installed = false;
    form.package = "network-manager-l2tp";
    VpnEditor editor;
    editor.load(form);
    EXPECT_FALSE(editor.installed());
    EXPECT_FALSE(editor.complete());
}

TEST(VpnEditor, WireGuardKeepsItsPeers)
{
    nm::VpnForm form;
    form.kind = "wireguard";
    form.installed = true;
    form.wireguard.addresses = "10.0.0.2/24";
    form.wireguard.peers = {{.publicKey = "xTIBA5rboUvnH4htodjb6e697QjLERt1NAB4mZqp8Dg=", .keepalive = "25"}};
    VpnEditor editor;
    editor.load(form);
    // The private key is stored, not shown: without it the form isn't complete.
    EXPECT_FALSE(editor.complete());
    nm::VpnSecrets secrets;
    secrets.privateKey = "yAnz5TF+lXXJte14tji3zlMNq+hd2rYUIgJBgB3fBmk=";
    editor.setSecrets(secrets);
    EXPECT_TRUE(editor.complete());

    const nm::VpnValues values = editor.collect();
    EXPECT_EQ(values.wireguard.privateKey, secrets.privateKey);
    EXPECT_EQ(values.wireguard.addresses, "10.0.0.2/24");
    ASSERT_EQ(values.wireguard.peers.size(), 1u);
    EXPECT_EQ(values.wireguard.peers[0].keepalive, "25");
    EXPECT_TRUE(values.wireguard.peerRoutes);
}

TEST(VpnEditor, RemovingAWireGuardPeerKeepsTheOthers)
{
    nm::VpnForm form;
    form.kind = "wireguard";
    form.installed = true;
    form.wireguard.peers = {{.publicKey = "first"}, {.publicKey = "second"}};
    Lingmo::ui::Window window;
    auto content = std::make_unique<VpnEditor>();
    VpnEditor* editor = content.get();
    window.setContentView(std::move(content));
    window.resize({480, 1200});
    editor->load(form);
    window.grab();

    Lingmo::ui::View* remove = nullptr;
    const std::function<void(Lingmo::ui::View*)> findRemove = [&](Lingmo::ui::View* view) {
        auto* label = dynamic_cast<Lingmo::ui::Label*>(view);
        if (!remove && label && label->text() == "Remove")
            remove = view->parent();
        for (const auto& child : view->children())
            findRemove(child.get());
    };
    findRemove(editor);
    ASSERT_NE(remove, nullptr);
    // The button goes with the peer's card it sits in.
    const Lingmo::ui::PointF origin = remove->mapToWindow({0, 0});
    Lingmo::ui::TestInput::click(window, {origin.x + remove->geometry().width / 2,
                                            origin.y + remove->geometry().height / 2});

    const nm::VpnValues values = editor->collect();
    ASSERT_EQ(values.wireguard.peers.size(), 1u);
    EXPECT_EQ(values.wireguard.peers[0].publicKey, "second");
}
