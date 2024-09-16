// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "dnotificationdndmodeconfig.h"
#include "fakedbus/notificationmanagerservice.h"

DNOTIFICATIONS_USE_NAMESPACE

class TestDNotificationDNDModeConfig: public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_fakeinter = new FakeNotificationManagerService();
        m_dconfig = new DNotificationDNDModeConfig(name);
    }
    static void TearDownTestCase()
    {
        delete m_fakeinter;
        delete m_dconfig;
    }
    void SetUp() override {}
    void TearDown() override {}

    static inline DNotificationDNDModeConfig *m_dconfig{nullptr};
    static inline FakeNotificationManagerService *m_fakeinter{nullptr};
    static inline QString name{"dnd"};
};

TEST_F(TestDNotificationDNDModeConfig, enabled)
{
    auto result = m_dconfig->enabled();
    EXPECT_EQ(result, m_fakeinter->m_enable);
}

TEST_F(TestDNotificationDNDModeConfig, setEnabled)
{
    auto result = m_dconfig->enabled();
    m_dconfig->setEnabled(!result);
    EXPECT_EQ(!result, m_fakeinter->m_enable);
}

TEST_F(TestDNotificationDNDModeConfig, DNDModeInLockScreenEnabled)
{
    auto result = m_dconfig->DNDModeInLockScreenEnabled();
    EXPECT_EQ(result, m_fakeinter->m_DNDModeInLockScreenEnabled);
}

TEST_F(TestDNotificationDNDModeConfig, setDNDModeInLockScreenEnabled)
{
    auto result = m_dconfig->DNDModeInLockScreenEnabled();
    m_dconfig->setDNDModeInLockScreenEnabled(!result);
    EXPECT_EQ(!result, m_fakeinter->m_DNDModeInLockScreenEnabled);
}

TEST_F(TestDNotificationDNDModeConfig, openByTimeIntervalEnabled)
{
    auto result = m_dconfig->openByTimeIntervalEnabled();
    EXPECT_EQ(result, m_fakeinter->m_openByTimeIntervalEnabled);
}

TEST_F(TestDNotificationDNDModeConfig, setOpenByTimeIntervalEnabled)
{
    auto result = m_dconfig->openByTimeIntervalEnabled();
    m_dconfig->setOpenByTimeIntervalEnabled(!result);
    EXPECT_EQ(!result, m_fakeinter->m_openByTimeIntervalEnabled);
}

TEST_F(TestDNotificationDNDModeConfig, startTime)
{
    auto result = m_dconfig->startTime();
    EXPECT_EQ(result, m_fakeinter->m_startTime);
}

TEST_F(TestDNotificationDNDModeConfig, setStartTime)
{
    auto result = m_dconfig->startTime();
    m_dconfig->setStartTime("10:10");
    EXPECT_EQ("10:10", m_fakeinter->m_startTime);
}

TEST_F(TestDNotificationDNDModeConfig, endTime)
{
    auto result = m_dconfig->endTime();
    EXPECT_EQ(result, m_fakeinter->m_endTime);
}

TEST_F(TestDNotificationDNDModeConfig, setEndTime)
{
    auto result = m_dconfig->endTime();
    m_dconfig->setEndTime("24:10");
    EXPECT_EQ("24:10", m_fakeinter->m_endTime);
}

TEST_F(TestDNotificationDNDModeConfig, showIconEnabled)
{
    auto result = m_dconfig->showIconEnabled();
    EXPECT_EQ(result, m_fakeinter->m_showIconEnabled);
}

TEST_F(TestDNotificationDNDModeConfig, setShowIconEnabled)
{
    auto result = m_dconfig->showIconEnabled();
    m_dconfig->setShowIconEnabled(!result);
    EXPECT_EQ(!result, m_fakeinter->m_showIconEnabled);
}
