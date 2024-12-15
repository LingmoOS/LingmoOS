// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "dactivevpnconnection.h"
#include "fakeDBus/activevpnconnectionservice.h"
#include "fakeDBus/networkmanagerservice.h"
#include <QDebug>

DNETWORKMANAGER_USE_NAMESPACE

class TestDActiveVpnConnection : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_nm = new FakeNetworkManagerService();
        m_fakeService = new FakeActiveVpnConncetionService();
        m_avc = new DActiveVpnConnection(2);
    }

    static void TearDownTestCase()
    {
        delete m_avc;
        delete m_fakeService;
        delete m_nm;
    }

    void SetUp() override {}
    void TearDown() override {}

    static inline DActiveVpnConnection *m_avc{nullptr};
    static inline FakeActiveVpnConncetionService *m_fakeService{nullptr};
    static inline FakeNetworkManagerService *m_nm{nullptr};
};

TEST_F(TestDActiveVpnConnection, devices)
{
    EXPECT_EQ(2, m_avc->devices()[0]);
}

TEST_F(TestDActiveVpnConnection, vpn)
{
    EXPECT_EQ(false, m_avc->vpn());
}

TEST_F(TestDActiveVpnConnection, connection)
{
    EXPECT_EQ(4, m_avc->connection());
}

TEST_F(TestDActiveVpnConnection, Dhcp4Config)
{
    EXPECT_EQ(2, m_avc->DHCP4Config());
}

TEST_F(TestDActiveVpnConnection, Dhcp6Config)
{
    EXPECT_EQ(0, m_avc->DHCP6Config());
}

TEST_F(TestDActiveVpnConnection, Ip4Config)
{
    EXPECT_EQ(4, m_avc->IP4Config());
}

TEST_F(TestDActiveVpnConnection, Ip6Config)
{
    EXPECT_EQ(4, m_avc->IP6Config());
}

TEST_F(TestDActiveVpnConnection, specificObject)
{
    EXPECT_EQ(0, m_avc->specificObject());
}

TEST_F(TestDActiveVpnConnection, id)
{
    EXPECT_EQ("有线连接", m_avc->connectionId());
}

TEST_F(TestDActiveVpnConnection, type)
{
    EXPECT_EQ(DNMSetting::SettingType::Wired, m_avc->connectionType());
}

TEST_F(TestDActiveVpnConnection, uuid)
{
    EXPECT_EQ(QString{"eb7e3c30-d5de-4a21-a697-1c31777b4276"}, m_avc->UUID().toString(QUuid::WithoutBraces));
}

TEST_F(TestDActiveVpnConnection, state)
{
    // using fully qualified to avoid ambiguous error
    EXPECT_EQ(Dtk::NetworkManager::NMActiveConnectionState::Activated, m_avc->connectionState());
}

TEST_F(TestDActiveVpnConnection, vpnState)
{
    EXPECT_EQ(Dtk::NetworkManager::NMVpnConnectionState::Activated, m_avc->vpnState());
}

TEST_F(TestDActiveVpnConnection, banner)
{
    EXPECT_EQ("welcome", m_avc->banner());
}
