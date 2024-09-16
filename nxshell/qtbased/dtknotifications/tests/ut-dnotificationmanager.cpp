// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "dnotificationmanager.h"
#include "fakedbus/notificationmanagerservice.h"

DNOTIFICATIONS_USE_NAMESPACE

class TestDNotificationManager : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_fakeinter = new FakeNotificationManagerService();
        m_damanager = new DNotificationManager();
    }
    static void TearDownTestCase()
    {
        delete m_fakeinter;
        delete m_damanager;
    }
    void SetUp() override {}
    void TearDown() override {}

    static inline DNotificationManager *m_damanager{nullptr};
    static inline FakeNotificationManagerService *m_fakeinter{nullptr};
};

TEST_F(TestDNotificationManager, allRecords)
{
    m_damanager->allRecords();
    EXPECT_EQ("test", m_fakeinter->m_getAllRecords);
}

TEST_F(TestDNotificationManager, getRecordsFromId)
{
    m_damanager->getRecordsFromId(1, "1920");
    EXPECT_EQ("test0, test1", m_fakeinter->m_records);
}

TEST_F(TestDNotificationManager, getRecordById)
{
    m_damanager->getRecordById("1920");
    EXPECT_EQ("test", m_fakeinter->m_record);
}

TEST_F(TestDNotificationManager, recordCount)
{
    m_damanager->recordCount();
    EXPECT_EQ(10, m_fakeinter->m_recordCount);
}

TEST_F(TestDNotificationManager, clearRecords)
{
    EXPECT_EQ(false, m_fakeinter->m_clearRecords);
    m_damanager->clearRecords();
    EXPECT_EQ(true, m_fakeinter->m_clearRecords);
}

TEST_F(TestDNotificationManager, removeRecord)
{
    EXPECT_EQ(false, m_fakeinter->m_removeRecord);
    m_damanager->removeRecord("1920");
    EXPECT_EQ(true, m_fakeinter->m_removeRecord);
}

TEST_F(TestDNotificationManager, serverInformation)
{
    m_damanager->serverInformation();
    EXPECT_EQ("deepin", m_fakeinter->m_serverInfo);
}

TEST_F(TestDNotificationManager, capbilities)
{
    m_damanager->capbilities();
    QStringList capabilities{"action-icons", "actions", "body", "body-hyperlinks", "body-markup"};
    EXPECT_EQ(capabilities, m_fakeinter->m_capabilities);
}

TEST_F(TestDNotificationManager, closeNotification)
{
    EXPECT_EQ(false, m_fakeinter->m_closeNotification);
    m_damanager->closeNotification(110);
    EXPECT_EQ(true, m_fakeinter->m_closeNotification);
}

TEST_F(TestDNotificationManager, appList)
{
    m_damanager->appList();
    QStringList appList{"dde-control-center", "dde-dock"};
    EXPECT_EQ(appList, m_fakeinter->m_appList);
}

TEST_F(TestDNotificationManager, appNotificationConfig)
{
    m_damanager->notificationAppConfig("id");
    QDBusVariant appInfo{QString("app")};
    EXPECT_EQ("app", m_fakeinter->m_appInfo.variant().toString());
}

TEST_F(TestDNotificationManager, dndModeNotificationConfig)
{
    m_damanager->notificationDNDModeConfig();
    EXPECT_EQ("sys", m_fakeinter->m_sysInfo.variant().toString());
}

TEST_F(TestDNotificationManager, toggleNotificationCenter)
{
    EXPECT_EQ(false, m_fakeinter->m_toggle);
    m_damanager->toggleNotificationCenter();
    EXPECT_EQ(true, m_fakeinter->m_toggle);
}

TEST_F(TestDNotificationManager, showNotificationCenter)
{
    EXPECT_EQ(false, m_fakeinter->m_show);
    m_damanager->showNotificationCenter();
    EXPECT_EQ(true, m_fakeinter->m_show);
}

TEST_F(TestDNotificationManager, hideNotificationCenter)
{
    EXPECT_EQ(false, m_fakeinter->m_hide);
    m_damanager->hideNotificationCenter();
    EXPECT_EQ(true, m_fakeinter->m_hide);
}
