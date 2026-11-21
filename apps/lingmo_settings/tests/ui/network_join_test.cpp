// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "lingmo/networkmanager/handler.h"
#include "pages/network/wlan/newnetworkdialog.h"
#include "ui/combo_box.h"
#include "ui/text_field.h"
#include "widgets/formdialog.h"
#include "window/settingswindow.h"

using namespace settings;
using namespace settings::network;
namespace nm = Lingmo::Networkmanager;

namespace {

struct JoinDialog : testing::Test
{
    void SetUp() override
    {
        window = std::make_unique<SettingsWindow>();
        window->reveal();
        dialog = std::make_unique<NewNetworkDialog>(*window);
        dialog->onJoinNetwork = [this](const std::string& device, const std::string& accessPoint,
                                       const std::string& userName, const std::string& password) {
            joins.push_back({device, accessPoint, userName, password});
        };
        dialog->onConnect = [this](const std::string& ssid, const std::string& userName, const std::string& password,
                                   const std::string& type) { connects.push_back({ssid, userName, password, type}); };
    }

    const nm::WirelessNetwork library{"/org/freedesktop/NetworkManager/Devices/2", "Library", nm::SecurityType::Wpa2Eap};
    const std::string libraryAccessPoint = "/org/freedesktop/NetworkManager/AccessPoint/5";

    std::unique_ptr<SettingsWindow> window;
    std::unique_ptr<NewNetworkDialog> dialog;
    std::vector<std::vector<std::string>> joins;
    std::vector<std::vector<std::string>> connects;
};

} // namespace

TEST_F(JoinDialog, JoinsAListedEnterpriseNetwork)
{
    dialog->openNetwork(libraryAccessPoint, library);
    ASSERT_TRUE(dialog->isOpen());
    EXPECT_EQ(dialog->title(), "Join “Library”");
    EXPECT_EQ(dialog->security(), "wpa2-eap");

    // The network is known; only the sign-in is asked for, user name first.
    EXPECT_FALSE(dialog->nameField()->isVisible());
    EXPECT_FALSE(dialog->securityBox()->isVisible());
    EXPECT_TRUE(dialog->userNameField()->isVisible());
    EXPECT_TRUE(dialog->passwordField()->isVisible());
    EXPECT_TRUE(dialog->userNameField()->hasFocus());

    EXPECT_FALSE(dialog->canJoin());
    dialog->userNameField()->setText("alice");
    EXPECT_FALSE(dialog->canJoin());
    dialog->passwordField()->setText("secret");
    EXPECT_TRUE(dialog->canJoin());

    dialog->join();
    ASSERT_EQ(joins.size(), 1u);
    EXPECT_EQ(joins[0], (std::vector<std::string>{library.device, libraryAccessPoint, "alice", "secret"}));
    EXPECT_TRUE(connects.empty());
    EXPECT_FALSE(dialog->isOpen());
}

TEST_F(JoinDialog, WpaEnterpriseKeepsItsKind)
{
    dialog->openNetwork(libraryAccessPoint, nm::WirelessNetwork{library.device, "Old", nm::SecurityType::WpaEap});
    ASSERT_TRUE(dialog->isOpen());
    EXPECT_EQ(dialog->security(), "wpa-eap");
}

TEST_F(JoinDialog, CertificateNetworkCannotBeJoined)
{
    dialog->openNetwork(libraryAccessPoint, nm::WirelessNetwork{library.device, "Secure", nm::SecurityType::Wpa3SuiteB192});
    ASSERT_TRUE(dialog->isOpen());
    EXPECT_TRUE(dialog->needsCertificate());
    EXPECT_FALSE(dialog->userNameField()->isVisible());
    EXPECT_FALSE(dialog->passwordField()->isVisible());

    dialog->userNameField()->setText("alice");
    dialog->passwordField()->setText("secret");
    EXPECT_FALSE(dialog->canJoin());
    dialog->join();
    EXPECT_TRUE(joins.empty());
}

TEST_F(JoinDialog, GoneNetworkOpensNothing)
{
    // Handler::wirelessNetwork() is empty once the access point has gone.
    dialog->openNetwork(libraryAccessPoint, std::nullopt);
    EXPECT_FALSE(dialog->isOpen());
}

TEST_F(JoinDialog, ClosingReturnsToHiddenNetworks)
{
    dialog->openNetwork(libraryAccessPoint, library);
    ASSERT_TRUE(dialog->isOpen());
    dialog->userNameField()->setText("alice");
    dialog->close();
    EXPECT_FALSE(dialog->isOpen());

    dialog->show();
    ASSERT_TRUE(dialog->isOpen());
    EXPECT_EQ(dialog->title(), "Add Other Network");
    EXPECT_EQ(dialog->security(), "wpa2-wpa3");
    EXPECT_EQ(dialog->userNameField()->text(), "");
    EXPECT_TRUE(dialog->nameField()->isVisible());
    EXPECT_TRUE(dialog->securityBox()->isVisible());
    EXPECT_TRUE(dialog->nameField()->hasFocus());
    dialog->nameField()->setText("Garden");
    dialog->passwordField()->setText("garden pass");
    dialog->join();
    ASSERT_EQ(connects.size(), 1u);
    EXPECT_EQ(connects[0], (std::vector<std::string>{"Garden", "", "garden pass", "wpa2-wpa3"}));
    EXPECT_TRUE(joins.empty());
}

TEST_F(JoinDialog, PasswordsFitTheSecurity)
{
    dialog->show();
    dialog->nameField()->setText("Garden");
    // WPA2/WPA3 passphrases are 8 to 63 characters.
    dialog->passwordField()->setText("short");
    EXPECT_FALSE(dialog->canJoin());
    dialog->passwordField()->setText("long enough");
    EXPECT_TRUE(dialog->canJoin());
    // 64 characters are the key itself, in hexadecimal.
    dialog->passwordField()->setText(std::string(64, 'a'));
    EXPECT_TRUE(dialog->canJoin());
    dialog->passwordField()->setText(std::string(64, 'z'));
    EXPECT_FALSE(dialog->canJoin());
    // WEP keys come in four lengths, the longer two hexadecimal; an open network needs none.
    dialog->securityBox()->setCurrentIndex(1);
    EXPECT_FALSE(dialog->canJoin());
    dialog->passwordField()->setText("abcde");
    EXPECT_TRUE(dialog->canJoin());
    dialog->passwordField()->setText("0123456789");
    EXPECT_TRUE(dialog->canJoin());
    dialog->passwordField()->setText("garden key");
    EXPECT_FALSE(dialog->canJoin());
    dialog->securityBox()->setCurrentIndex(0);
    dialog->passwordField()->setText("");
    EXPECT_TRUE(dialog->canJoin());
}
