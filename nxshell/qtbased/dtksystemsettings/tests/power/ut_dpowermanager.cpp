// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "dpowerdevice.h"
#include "dpowermanager.h"
#include "upowermanagerservice.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>

#include <iostream>

#include <unistd.h>
DPOWER_USE_NAMESPACE

class TestDPowerManager : public testing::Test
{
public:
    // 在测试套件的第一个测试用例开始前，SetUpTestCase 函数会被调用
    static void SetUpTestCase()
    {
        m_fakeInterface = new UPowerManagerService();
        m_dpowermanager = new DPowerManager();
    }

    // 在测试套件中的最后一个测试用例运行结束后，TearDownTestCase 函数会被调用
    static void TearDownTestCase()
    {
        delete m_fakeInterface;
        delete m_dpowermanager;
        m_fakeInterface = nullptr;
        m_dpowermanager = nullptr;
    }

    // 每个测试用例开始前，SetUp 函数都会被被调用
    void SetUp() override { }

    // 每个测试用例运行结束后，TearDown 函数都会被被调用
    void TearDown() override { }

    static UPowerManagerService *m_fakeInterface;
    static DPowerManager *m_dpowermanager;
};

UPowerManagerService *TestDPowerManager::m_fakeInterface = nullptr;
DPowerManager *TestDPowerManager::m_dpowermanager = nullptr;

QT_BEGIN_NAMESPACE // for QString support
        inline void
        PrintTo(const QString &qString, ::std::ostream *os)
{
    *os << qUtf8Printable(qString);
}

QT_END_NAMESPACE

TEST_F(TestDPowerManager, displayDevice)
{
    auto ddevice = m_dpowermanager->displayDevice();
    ASSERT_EQ("DisplayDevice", ddevice->deviceName());
}

TEST_F(TestDPowerManager, criticalAction)
{
    ASSERT_EQ("true", m_dpowermanager->criticalAction());
}

TEST_F(TestDPowerManager, devices)
{
    QStringList list = m_dpowermanager->devices().value();
    ASSERT_EQ("battery_BAT1", list[0]);
}

TEST_F(TestDPowerManager, properties)
{
    EXPECT_FALSE(m_dpowermanager->lidIsClosed());
    EXPECT_FALSE(m_dpowermanager->lidIsPresent());
    EXPECT_EQ("YES", m_dpowermanager->daemonVersion());
}
