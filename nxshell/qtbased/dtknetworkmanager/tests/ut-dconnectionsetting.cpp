// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "dconnectionsetting.h"
#include "fakeDBus/connectionsettingservice.h"
#include "fakeDBus/networkmanagerservice.h"

DNETWORKMANAGER_USE_NAMESPACE

class TestDConnectionSetting : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_nm = new FakeNetworkManagerService();
        m_fakeService = new FakeConnectionSettingService();
        m_cs = new DConnectionSetting(3);
    }

    static void TearDownTestCase()
    {
        delete m_cs;
        delete m_fakeService;
        delete m_nm;
    }

    void SetUp() override {}
    void TearDown() override {}

    static inline DConnectionSetting *m_cs{nullptr};
    static inline FakeConnectionSettingService *m_fakeService{nullptr};
    static inline FakeNetworkManagerService *m_nm{nullptr};
};

TEST_F(TestDConnectionSetting, unsaved)
{
    EXPECT_EQ(true, m_cs->unsaved());
}

TEST_F(TestDConnectionSetting, filename)
{
    EXPECT_EQ("/etc/NetworkManager/system-connections/uniontech.nmconnection", m_cs->filename().toLocalFile());
}

TEST_F(TestDConnectionSetting, flags)
{
    EXPECT_EQ(true, m_cs->flags().testFlag(NMSettingsConnectionFlags::FlagUnsaved));
    EXPECT_EQ(true, m_cs->flags().testFlag(NMSettingsConnectionFlags::NMGenerated));
}

TEST_F(TestDConnectionSetting, Update)
{
    auto ret = m_cs->updateSetting({});
    if (!ret)
        qWarning() << ret.error();
    EXPECT_EQ(true, m_fakeService->m_updateSettingTrigger);
}

TEST_F(TestDConnectionSetting, Delete)
{
    auto ret = m_cs->deleteSetting();
    if (!ret)
        qWarning() << ret.error();
    EXPECT_EQ(true, m_fakeService->m_deleteSettingTrigger);
}

TEST_F(TestDConnectionSetting, GetSettings)
{
    auto ret = m_cs->settings();
    if (!ret)
        qWarning() << ret.error();
    EXPECT_EQ(true, m_fakeService->m_updateSettingTrigger);
}
