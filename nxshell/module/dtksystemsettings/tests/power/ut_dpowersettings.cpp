// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "daemonpowerservice.h"
#include "dpowersettings.h"
#include "dpowertypes.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>

DPOWER_USE_NAMESPACE

class TestDPowerSettings : public testing::Test
{
public:
    // 在测试套件的第一个测试用例开始前，SetUpTestCase 函数会被调用
    static void SetUpTestCase()
    {
        m_fakeInterface = new DaemonPowerService();
        m_dpowerSettings = new DPowerSettings();
    }

    // 在测试套件中的最后一个测试用例运行结束后，TearDownTestCase 函数会被调用
    static void TearDownTestCase()
    {
        delete m_fakeInterface;
        delete m_dpowerSettings;
        m_fakeInterface = nullptr;
        m_dpowerSettings = nullptr;
    }

    // 每个测试用例开始前，SetUp 函数都会被被调用
    void SetUp() override { }

    // 每个测试用例运行结束后，TearDown 函数都会被被调用
    void TearDown() override { }

    static DaemonPowerService *m_fakeInterface;
    static DPowerSettings *m_dpowerSettings;
};

DaemonPowerService *TestDPowerSettings::m_fakeInterface = nullptr;
DPowerSettings *TestDPowerSettings::m_dpowerSettings = nullptr;

TEST_F(TestDPowerSettings, reset)
{
    EXPECT_EQ(false, m_fakeInterface->m_reset);

    m_dpowerSettings->reset();

    EXPECT_EQ(true, m_fakeInterface->m_reset);
}

TEST_F(TestDPowerSettings, batteryLidClosedAction)
{
    m_dpowerSettings->setBatteryLidClosedAction(LidClosedAction::TurnoffScreen);
    ASSERT_EQ(3, m_fakeInterface->m_batteryLidClosedAction);
    EXPECT_EQ(LidClosedAction::TurnoffScreen, m_dpowerSettings->batteryLidClosedAction());
    m_fakeInterface->setBatteryLidClosedAction(2);
    ASSERT_EQ(2, m_fakeInterface->m_batteryLidClosedAction);
    EXPECT_EQ(LidClosedAction::Hibernate, m_dpowerSettings->batteryLidClosedAction());
}

TEST_F(TestDPowerSettings, batteryLockDelay)
{
    m_dpowerSettings->setBatteryLockDelay(100);
    ASSERT_EQ(100, m_fakeInterface->m_batteryLockDelay);
    EXPECT_EQ(100, m_dpowerSettings->batteryLockDelay());
    m_fakeInterface->setBatteryLockDelay(200);
    ASSERT_EQ(200, m_fakeInterface->m_batteryLockDelay);
    EXPECT_EQ(200, m_dpowerSettings->batteryLockDelay());
}

TEST_F(TestDPowerSettings, batteryPressPowerBtnAction)
{
    m_dpowerSettings->setBatteryPressPowerBtnAction(PowerBtnAction::Suspend);
    ASSERT_EQ(1, m_fakeInterface->m_batteryPressPowerBtnAction);
    EXPECT_EQ(PowerBtnAction::Suspend, m_dpowerSettings->batteryPressPowerBtnAction());
    m_fakeInterface->setBatteryPressPowerBtnAction(2);
    EXPECT_EQ(2, m_fakeInterface->m_batteryPressPowerBtnAction);
    EXPECT_EQ(PowerBtnAction::Hibernate, m_dpowerSettings->batteryPressPowerBtnAction());
}

TEST_F(TestDPowerSettings, batteryScreenBlackDelay)
{
    m_dpowerSettings->setBatteryScreenBlackDelay(100);
    ASSERT_EQ(100, m_fakeInterface->m_batteryScreenBlackDelay);
    EXPECT_EQ(100, m_dpowerSettings->batteryScreenBlackDelay());
    m_fakeInterface->setBatteryScreenBlackDelay(200);
    ASSERT_EQ(200, m_fakeInterface->m_batteryScreenBlackDelay);
    EXPECT_EQ(200, m_dpowerSettings->batteryScreenBlackDelay());
}

TEST_F(TestDPowerSettings, BatteryScreenBlackDelay)
{
    m_dpowerSettings->setBatteryScreensaverDelay(100);
    ASSERT_EQ(100, m_fakeInterface->m_batteryScreensaverDelay);
    EXPECT_EQ(100, m_dpowerSettings->batteryScreensaverDelay());
    m_fakeInterface->setBatteryScreensaverDelay(200);
    ASSERT_EQ(200, m_fakeInterface->m_batteryScreensaverDelay);
    EXPECT_EQ(200, m_dpowerSettings->batteryScreensaverDelay());
}

TEST_F(TestDPowerSettings, batterySleepDelay)
{
    m_dpowerSettings->setBatterySleepDelay(100);
    ASSERT_EQ(100, m_fakeInterface->m_batterySleepDelay);
    EXPECT_EQ(100, m_dpowerSettings->batterySleepDelay());
    m_fakeInterface->setBatterySleepDelay(200);
    ASSERT_EQ(200, m_fakeInterface->m_batterySleepDelay);
    EXPECT_EQ(200, m_dpowerSettings->batterySleepDelay());
}

TEST_F(TestDPowerSettings, linePowerLidClosedAction)
{
    m_dpowerSettings->setLinePowerLidClosedAction(LidClosedAction::Suspend);
    ASSERT_EQ(1, m_fakeInterface->m_linePowerLidClosedAction);
    EXPECT_EQ(LidClosedAction::Suspend, m_dpowerSettings->linePowerLidClosedAction());
    m_fakeInterface->setLinePowerLidClosedAction(2);
    EXPECT_EQ(2, m_fakeInterface->m_linePowerLidClosedAction);
    EXPECT_EQ(LidClosedAction::Hibernate, m_dpowerSettings->linePowerLidClosedAction());
}

TEST_F(TestDPowerSettings, linePowerLockDelay)
{
    m_dpowerSettings->setLinePowerLockDelay(100);
    ASSERT_EQ(100, m_fakeInterface->m_linePowerLockDelay);
    EXPECT_EQ(100, m_dpowerSettings->linePowerLockDelay());
    m_fakeInterface->setLinePowerLockDelay(200);
    ASSERT_EQ(200, m_fakeInterface->m_linePowerLockDelay);
    EXPECT_EQ(200, m_dpowerSettings->linePowerLockDelay());
}

TEST_F(TestDPowerSettings, linePowerPressPowerBtnAction)
{
    m_dpowerSettings->setLinePowerPressPowerBtnAction(PowerBtnAction::Suspend);
    ASSERT_EQ(1, m_fakeInterface->m_linePowerPressPowerBtnAction);
    EXPECT_EQ(PowerBtnAction::Suspend, m_dpowerSettings->linePowerPressPowerBtnAction());
    m_fakeInterface->setLinePowerPressPowerBtnAction(2);
    ASSERT_EQ(2, m_fakeInterface->m_linePowerPressPowerBtnAction);
    EXPECT_EQ(PowerBtnAction::Hibernate, m_dpowerSettings->linePowerPressPowerBtnAction());
}

TEST_F(TestDPowerSettings, linePowerScreenBlackDelay)
{
    m_dpowerSettings->setLinePowerScreenBlackDelay(100);
    ASSERT_EQ(100, m_fakeInterface->m_linePowerScreenBlackDelay);
    EXPECT_EQ(100, m_dpowerSettings->linePowerScreenBlackDelay());
    m_fakeInterface->setLinePowerScreenBlackDelay(200);
    ASSERT_EQ(200, m_fakeInterface->m_linePowerScreenBlackDelay);
    EXPECT_EQ(200, m_dpowerSettings->linePowerScreenBlackDelay());
}

TEST_F(TestDPowerSettings, linePowerScreensaverDelay)
{
    m_dpowerSettings->setLinePowerScreensaverDelay(100);
    ASSERT_EQ(100, m_fakeInterface->m_linePowerScreensaverDelay);
    EXPECT_EQ(100, m_dpowerSettings->linePowerScreensaverDelay());
    m_fakeInterface->setLinePowerScreensaverDelay(200);
    ASSERT_EQ(200, m_fakeInterface->m_linePowerScreensaverDelay);
    EXPECT_EQ(200, m_dpowerSettings->linePowerScreensaverDelay());
}

TEST_F(TestDPowerSettings, linePowerSleepDelay)
{
    m_dpowerSettings->setLinePowerSleepDelay(100);
    ASSERT_EQ(100, m_fakeInterface->m_linePowerSleepDelay);
    EXPECT_EQ(100, m_dpowerSettings->linePowerSleepDelay());
    m_fakeInterface->setLinePowerSleepDelay(200);
    ASSERT_EQ(200, m_fakeInterface->m_linePowerSleepDelay);
    EXPECT_EQ(200, m_dpowerSettings->linePowerSleepDelay());
}

TEST_F(TestDPowerSettings, lowPowerAutoSleepThreshold)
{
    m_dpowerSettings->setLowPowerAutoSleepThreshold(5);
    ASSERT_EQ(5, m_fakeInterface->m_lowPowerAutoSleepThreshold);
    EXPECT_EQ(5, m_dpowerSettings->lowPowerAutoSleepThreshold());
    m_fakeInterface->setLowPowerAutoSleepThreshold(2);
    ASSERT_EQ(2, m_fakeInterface->m_lowPowerAutoSleepThreshold);
    EXPECT_EQ(2, m_dpowerSettings->lowPowerAutoSleepThreshold());
}

TEST_F(TestDPowerSettings, lowPowerNotifyEnable)
{
    m_dpowerSettings->setLowPowerNotifyEnable(true);
    ASSERT_TRUE(m_fakeInterface->m_lowPowerNotifyEnable);
    EXPECT_TRUE(m_dpowerSettings->lowPowerNotifyEnable());
    m_fakeInterface->setLowPowerNotifyEnable(false);
    ASSERT_FALSE(m_fakeInterface->m_lowPowerNotifyEnable);
    EXPECT_FALSE(m_dpowerSettings->lowPowerNotifyEnable());
}

TEST_F(TestDPowerSettings, lowPowerNotifyThreshold)
{
    m_dpowerSettings->setLowPowerNotifyThreshold(25);
    ASSERT_EQ(25, m_fakeInterface->m_lowPowerNotifyThreshold);
    EXPECT_EQ(25, m_dpowerSettings->lowPowerNotifyThreshold());
    m_fakeInterface->setLowPowerNotifyThreshold(10);
    ASSERT_EQ(10, m_fakeInterface->m_lowPowerNotifyThreshold);
    EXPECT_EQ(10, m_dpowerSettings->lowPowerNotifyThreshold());
}

TEST_F(TestDPowerSettings, screenBlackLock)
{
    m_dpowerSettings->setScreenBlackLock(true);
    ASSERT_TRUE(m_fakeInterface->m_screenBlackLock);
    EXPECT_TRUE(m_dpowerSettings->screenBlackLock());
    m_fakeInterface->setScreenBlackLock(false);
    ASSERT_FALSE(m_fakeInterface->m_screenBlackLock);
    EXPECT_FALSE(m_dpowerSettings->screenBlackLock());
}

TEST_F(TestDPowerSettings, sleepLock)
{
    m_dpowerSettings->setSleepLock(true);
    ASSERT_TRUE(m_fakeInterface->m_sleepLock);
    EXPECT_TRUE(m_dpowerSettings->sleepLock());
    m_fakeInterface->setSleepLock(false);
    ASSERT_FALSE(m_fakeInterface->m_sleepLock);
    EXPECT_FALSE(m_dpowerSettings->sleepLock());
}

/*

    Q_PROPERTY(bool SleepLock READ sleepLock WRITE setSleepLock);
*/
