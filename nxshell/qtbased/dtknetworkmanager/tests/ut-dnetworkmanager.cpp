// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "dnetworkmanager.h"
#include "fakeDBus/networkmanagerservice.h"

DNETWORKMANAGER_USE_NAMESPACE

class TestDNetworkManager : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_fakeService = new FakeNetworkManagerService();
        m_mg = new DNetworkManager();
    }

    static void TearDownTestCase()
    {
        delete m_fakeService;
        delete m_mg;
    }

    void SetUp() override {}
    void TearDown() override {}

    static inline DNetworkManager *m_mg{nullptr};
    static inline FakeNetworkManagerService *m_fakeService{nullptr};
};

TEST_F(TestDNetworkManager, networkingEnabled)
{
    EXPECT_EQ(true, m_mg->networkingEnabled());
}

TEST_F(TestDNetworkManager, wirelessEnabled)
{
    EXPECT_EQ(true, m_mg->wirelessEnabled());
    m_mg->setWirelessEnabled(false);
    EXPECT_EQ(false, m_mg->wirelessEnabled());
}

TEST_F(TestDNetworkManager, wirelessHardwareEnabled)
{
    EXPECT_EQ(true, m_mg->wirelessHardwareEnabled());
}

TEST_F(TestDNetworkManager, getActiveConnectionsIdList)
{
    EXPECT_EQ(1, m_mg->getActiveConnectionsIdList()[0]);
}

TEST_F(TestDNetworkManager, primaryConnection)
{
    EXPECT_EQ(1, m_mg->primaryConnection());
}

TEST_F(TestDNetworkManager, primaryConnectionType)
{
    EXPECT_EQ(DNMSetting::SettingType::Wired, m_mg->primaryConnectionType());
}

TEST_F(TestDNetworkManager, state)
{
    EXPECT_EQ(Dtk::NetworkManager::NMState::ConnectedGlobal, m_mg->state());
}

TEST_F(TestDNetworkManager, connectivity)
{
    EXPECT_EQ(Dtk::NetworkManager::NMConnectivityState::Full, m_mg->connectivity());
}

TEST_F(TestDNetworkManager, getDeviceIdList)
{
    auto ret = m_mg->getDeviceIdList();
    if (!ret)
        qWarning() << ret.error();
    EXPECT_EQ(true, m_fakeService->m_getDevicesTrigger);
}

TEST_F(TestDNetworkManager, activateConnection)
{
    auto ret = m_mg->activateConnection(1, 2);
    if (!ret)
        qWarning() << ret.error();
    EXPECT_EQ(true, m_fakeService->m_activateConnectionTrigger);
}

TEST_F(TestDNetworkManager, addAndActivateConnection)
{
    auto ret = m_mg->addAndActivateConnection({}, 2);
    if (!ret)
        qWarning() << ret.error();
    EXPECT_EQ(true, m_fakeService->m_addAndActivateConnectionTrigger);
}

TEST_F(TestDNetworkManager, deactivateConnection)
{
    auto ret = m_mg->deactivateConnection(1);
    if (!ret)
        qWarning() << ret.error();
    EXPECT_EQ(true, m_fakeService->m_deactivateConnectionTrigger);
}

TEST_F(TestDNetworkManager, enable)
{
    auto ret = m_mg->enable(true);
    if (!ret)
        qWarning() << ret.error();
    EXPECT_EQ(true, m_fakeService->m_enableTrigger);
}

TEST_F(TestDNetworkManager, permissions)
{
    auto ret = m_mg->permissions();
    if (!ret)
        qWarning() << ret.error();
    EXPECT_EQ(true, m_fakeService->m_getPermissionsTrigger);
}

TEST_F(TestDNetworkManager, checkConnectivity)
{
    auto ret = m_mg->checkConnectivity();
    if (!ret)
        qWarning() << ret.error();
    EXPECT_EQ(true, m_fakeService->m_checkConnectivityTrigger);
}
