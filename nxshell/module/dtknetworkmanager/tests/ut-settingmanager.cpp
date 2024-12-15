// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "dsettingsmanager.h"
#include "fakeDBus/networkmanagerservice.h"
#include "fakeDBus/settingsservice.h"

DNETWORKMANAGER_USE_NAMESPACE

class TestDSettingsManager : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_nm = new FakeNetworkManagerService();
        m_fakeService = new FakeSettingsService();
        m_sm = new DSettingsManager();
    }

    static void TearDownTestCase()
    {
        delete m_sm;
        delete m_fakeService;
        delete m_nm;
    }

    void SetUp() override {}
    void TearDown() override {}

    static inline DSettingsManager *m_sm{nullptr};
    static inline FakeSettingsService *m_fakeService{nullptr};
    static inline FakeNetworkManagerService *m_nm{nullptr};
};

TEST_F(TestDSettingsManager, listConnections)
{
    auto ret = m_sm->listConnections();
    if (!ret)
        qWarning() << ret.error();
    EXPECT_EQ(true, m_fakeService->m_listConnectionsTrigger);
}

TEST_F(TestDSettingsManager, getConnectionByUUID)
{
    auto ret = m_sm->getConnectionByUUID({});
    if (!ret)
        qWarning() << ret.error();
    EXPECT_EQ(true, m_fakeService->m_getConnectionByUUIDTrigger);
}

TEST_F(TestDSettingsManager, addConnection)
{
    auto ret = m_sm->addConnection({});
    if (!ret)
        qWarning() << ret.error();
    EXPECT_EQ(true, m_fakeService->m_addConnectionTrigger);
}
