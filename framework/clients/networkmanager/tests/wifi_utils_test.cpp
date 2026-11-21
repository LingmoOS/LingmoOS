// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "lingmo/networkmanager/wifi_utils.h"

using namespace Lingmo::Networkmanager;

TEST(WifiUtils, SsidLabels)
{
    EXPECT_EQ(wifi::ssidLabel("Caf\xc3\xa9"), "Caf\xc3\xa9");
    // GB18030 bytes of two Han characters, which a zh_CN system tries after UTF-8.
    EXPECT_EQ(wifi::ssidLabel("\xd6\xd0\xce\xc4", "zh_CN"), "\xe4\xb8\xad\xe6\x96\x87");
    EXPECT_EQ(wifi::ssidLabel(std::string("Lab\0", 4)), "Lab");
    EXPECT_EQ(wifi::ssidLabel("a\nb"), "a\xef\xbf\xbd" "b");
    // Without a matching charset the bytes read as Latin-1.
    EXPECT_EQ(wifi::ssidLabel("\xe9t\xe9", "en_US"), "\xc3\xa9t\xc3\xa9");
}

TEST(WifiUtils, HiddenSsids)
{
    EXPECT_TRUE(wifi::ssidHidden(""));
    EXPECT_TRUE(wifi::ssidHidden(std::string("\0\0\0", 3)));
    EXPECT_TRUE(wifi::ssidHidden(" "));
    EXPECT_FALSE(wifi::ssidHidden("  "));
    EXPECT_FALSE(wifi::ssidHidden("Home"));
}

TEST(WifiUtils, Security)
{
    // WEP40, WEP104, TKIP, CCMP, WPA and RSN.
    const uint32_t caps = 0x1 | 0x2 | 0x4 | 0x8 | 0x10 | 0x20;
    const uint32_t ccmp = 0x8 | 0x80;
    EXPECT_EQ(wifi::security(caps, 0, 0, 0, 2), SecurityType::None);
    EXPECT_EQ(wifi::security(caps, 1, 0, 0, 2), SecurityType::StaticWep);
    EXPECT_EQ(wifi::security(caps, 1, 0, ccmp | 0x100, 2), SecurityType::Wpa2Psk);
    EXPECT_EQ(wifi::security(caps, 1, 0, ccmp | 0x100 | 0x400, 2), SecurityType::Sae);
    EXPECT_EQ(wifi::security(caps, 1, 0, ccmp | 0x200, 2), SecurityType::Wpa2Eap);
    EXPECT_EQ(wifi::security(caps, 0, 0, 0x1000, 2), SecurityType::Owe);
    EXPECT_EQ(wifi::security(caps, 1, 0x4 | 0x40 | 0x100, 0, 2), SecurityType::WpaPsk);
    // A device that reports no capabilities still gets a lock for a secured network.
    EXPECT_EQ(wifi::security(0, 1, 0, ccmp | 0x100, 2), SecurityType::Wpa2Psk);
}

TEST(WifiUtils, Secrets)
{
    EXPECT_TRUE(wifi::pskValid("12345678"));
    EXPECT_FALSE(wifi::pskValid("1234567"));
    EXPECT_TRUE(wifi::pskValid(std::string(64, 'a')));
    EXPECT_FALSE(wifi::pskValid(std::string(64, 'g')));
    EXPECT_EQ(wifi::generatedPassword().size(), 12u);
    EXPECT_TRUE(wifi::pskValid(wifi::generatedPassword()));
    EXPECT_NE(wifi::generatedPassword(), wifi::generatedPassword());
}

TEST(WifiUtils, Strength)
{
    EXPECT_EQ(wifi::strengthIcon(0), "network-wireless-connected-00");
    EXPECT_EQ(wifi::strengthIcon(33), "network-wireless-connected-33");
    EXPECT_EQ(wifi::strengthIcon(34), "network-wireless-connected-66");
    EXPECT_EQ(wifi::strengthIcon(90), "network-wireless-connected-100");
}

TEST(WifiUtils, SsidBytesCutOnACharacter)
{
    const std::string name = std::string(31, 'a') + "\xe4\xb8\xad";
    EXPECT_EQ(wifi::ssidBytes(name), std::string(31, 'a'));
    EXPECT_EQ(wifi::ssidBytes("Home"), "Home");
}
