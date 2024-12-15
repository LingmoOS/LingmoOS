
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "dkbdbacklight.h"
#include "upowerkbdbacklightservice.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>

#include <unistd.h>
DPOWER_USE_NAMESPACE

class TestDPowerKbdBacklight : public testing::Test
{
public:
    // 在测试套件的第一个测试用例开始前，SetUpTestCase 函数会被调用
    static void SetUpTestCase()
    {
        m_fakeInterface = new UPowerKbdBacklightService();
        m_dkbdBackLight = new DKbdBacklight();
    }

    // 在测试套件中的最后一个测试用例运行结束后，TearDownTestCase 函数会被调用
    static void TearDownTestCase()
    {
        delete m_fakeInterface;
        delete m_dkbdBackLight;
        m_fakeInterface = nullptr;
        m_dkbdBackLight = nullptr;
    }

    // 每个测试用例开始前，SetUp 函数都会被被调用
    void SetUp() override { }

    // 每个测试用例运行结束后，TearDown 函数都会被被调用
    void TearDown() override { }

    static UPowerKbdBacklightService *m_fakeInterface;
    static DKbdBacklight *m_dkbdBackLight;
};

UPowerKbdBacklightService *TestDPowerKbdBacklight::m_fakeInterface = nullptr;
DKbdBacklight *TestDPowerKbdBacklight::m_dkbdBackLight = nullptr;

TEST_F(TestDPowerKbdBacklight, brightness)
{
    ASSERT_EQ(0, m_dkbdBackLight->brightness());
}

TEST_F(TestDPowerKbdBacklight, maxBrightness)
{
    ASSERT_EQ(uint(255), m_dkbdBackLight->maxBrightness());
}

TEST_F(TestDPowerKbdBacklight, setBrightness)
{
    m_dkbdBackLight->setBrightness(1);
    ASSERT_TRUE(m_fakeInterface->m_reset);
}
