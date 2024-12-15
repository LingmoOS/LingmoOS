// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "dnotificationappconfig.h"
#include "fakedbus/notificationmanagerservice.h"

DNOTIFICATIONS_USE_NAMESPACE

class TestDNotificationAppConfig: public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_fakeinter = new FakeNotificationManagerService();
        m_dconfig = new DNotificationAppConfig(id);
    }
    static void TearDownTestCase()
    {
        delete m_fakeinter;
        delete m_dconfig;
    }
    void SetUp() override {}
    void TearDown() override {}

    static inline DNotificationAppConfig *m_dconfig{nullptr};
    static inline FakeNotificationManagerService *m_fakeinter{nullptr};
    static inline QString id{"2022"};
};

TEST_F(TestDNotificationAppConfig, appName)
{
    auto result = m_dconfig->appName();
    EXPECT_EQ(result, m_fakeinter->m_appName);
}

TEST_F(TestDNotificationAppConfig, appIcon)
{
    auto result = m_dconfig->appIcon();
    EXPECT_EQ(result, m_fakeinter->m_appIcon);
}

TEST_F(TestDNotificationAppConfig, previewEnabled)
{
    auto result = m_dconfig->previewEnabled();
    EXPECT_EQ(result, m_fakeinter->m_enablePreview);
}

TEST_F(TestDNotificationAppConfig, setPreviewEnabled)
{
    auto result = m_dconfig->previewEnabled();
    m_dconfig->setPreviewEnabled(!result);
    EXPECT_EQ(!result, m_fakeinter->m_enablePreview);
}

TEST_F(TestDNotificationAppConfig, soundEnabled)
{
    auto result = m_dconfig->soundEnabled();
    EXPECT_EQ(result, m_fakeinter->m_enableSound);
}

TEST_F(TestDNotificationAppConfig, setSoundEnabled)
{
    auto result = m_dconfig->soundEnabled();
    m_dconfig->setSoundEnabled(!result);
    EXPECT_EQ(!result, m_fakeinter->m_enableSound);
}

TEST_F(TestDNotificationAppConfig, showInNotificationCenterEnabled)
{
    auto result = m_dconfig->showInNotificationCenterEnabled();
    EXPECT_EQ(result, m_fakeinter->m_showInNotification);
}

TEST_F(TestDNotificationAppConfig, setShowInNotificationCenterEnabled)
{
    auto result = m_dconfig->showInNotificationCenterEnabled();
    m_dconfig->setShowInNotificationCenterEnabled(!result);
    EXPECT_EQ(!result, m_fakeinter->m_showInNotification);
}

TEST_F(TestDNotificationAppConfig, showInLockScreenEnabled)
{
    auto result = m_dconfig->showInLockScreenEnabled();
    EXPECT_EQ(result, m_fakeinter->m_lockScreenShowNotification);
}

TEST_F(TestDNotificationAppConfig, setShowInLockScreenEnabled)
{
    auto result = m_dconfig->showInLockScreenEnabled();
    m_dconfig->setShowInLockScreenEnabled(!result);
    EXPECT_EQ(!result, m_fakeinter->m_showInNotification);
}
