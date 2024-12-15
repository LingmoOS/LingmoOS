// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "dpowerdevice.h"
#include "upowerdeviceservice.h"
#include "upowertypes_p.h"

#include <qdatetime.h>
#include <qdebug.h>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>

DPOWER_USE_NAMESPACE

class TestDPowerDevice : public testing::Test
{
public:
    // 在测试套件的第一个测试用例开始前，SetUpTestCase 函数会被调用
    static void SetUpTestCase()
    {
        m_fakeInterface = new UPowerDeviceService();
        m_dpowerDevice = new DPowerDevice("display");
    }

    // 在测试套件中的最后一个测试用例运行结束后，TearDownTestCase 函数会被调用
    static void TearDownTestCase()
    {
        delete m_fakeInterface;
        delete m_dpowerDevice;
        m_fakeInterface = nullptr;
        m_dpowerDevice = nullptr;
    }

    // 每个测试用例开始前，SetUp 函数都会被被调用
    void SetUp() override { }

    // 每个测试用例运行结束后，TearDown 函数都会被被调用
    void TearDown() override { }

    static UPowerDeviceService *m_fakeInterface;
    static DPowerDevice *m_dpowerDevice;
};

QT_BEGIN_NAMESPACE // for QString support
        inline void
        PrintTo(const QString &qString, ::std::ostream *os)
{
    *os << qUtf8Printable(qString);
}

QT_END_NAMESPACE

UPowerDeviceService *TestDPowerDevice::m_fakeInterface = nullptr;
DPowerDevice *TestDPowerDevice::m_dpowerDevice = nullptr;

TEST_F(TestDPowerDevice, hasHistory)
{
    ASSERT_FALSE(m_fakeInterface->hasHistory());
    ASSERT_FALSE(m_dpowerDevice->hasHistory());
}

TEST_F(TestDPowerDevice, hasStatistics)
{
    ASSERT_TRUE(m_fakeInterface->hasStatistics());
    ASSERT_TRUE(m_dpowerDevice->hasStatistics());
}

TEST_F(TestDPowerDevice, isPresent)
{
    ASSERT_TRUE(m_fakeInterface->isPresent());
    ASSERT_TRUE(m_dpowerDevice->isPresent());
}

TEST_F(TestDPowerDevice, isRechargeable)
{
    ASSERT_TRUE(m_fakeInterface->isRechargeable());
    ASSERT_TRUE(m_dpowerDevice->isRechargeable());
}

TEST_F(TestDPowerDevice, online)
{
    ASSERT_TRUE(m_fakeInterface->online());
    ASSERT_TRUE(m_dpowerDevice->online());
}

TEST_F(TestDPowerDevice, powerSupply)
{
    ASSERT_TRUE(m_fakeInterface->powerSupply());
    ASSERT_TRUE(m_dpowerDevice->powerSupply());
}

TEST_F(TestDPowerDevice, capacity)
{
    ASSERT_EQ(70, m_fakeInterface->capacity());
    ASSERT_EQ(70, m_dpowerDevice->capacity());
}

TEST_F(TestDPowerDevice, energyEmpty)
{
    ASSERT_EQ(0, m_fakeInterface->energyEmpty());
    ASSERT_EQ(0, m_dpowerDevice->energyEmpty());
}

TEST_F(TestDPowerDevice, energyFull)
{
    ASSERT_EQ(100, m_fakeInterface->energyFull());
    ASSERT_EQ(100, m_dpowerDevice->energyFull());
}

TEST_F(TestDPowerDevice, energyFullDesign)
{
    ASSERT_EQ(33.8, m_fakeInterface->energyFullDesign());
    ASSERT_EQ(33.8, m_dpowerDevice->energyFullDesign());
}

TEST_F(TestDPowerDevice, energyRate)
{
    ASSERT_EQ(99.7, m_fakeInterface->energyRate());
    ASSERT_EQ(99.7, m_dpowerDevice->energyRate());
}

TEST_F(TestDPowerDevice, luminosity)
{
    ASSERT_EQ(10, m_fakeInterface->luminosity());
    ASSERT_EQ(10, m_dpowerDevice->luminosity());
}

TEST_F(TestDPowerDevice, percentage)
{
    ASSERT_EQ(70, m_fakeInterface->percentage());
    ASSERT_EQ(70, m_dpowerDevice->percentage());
}

TEST_F(TestDPowerDevice, temperature)
{
    ASSERT_EQ(33.5, m_fakeInterface->temperature());
    ASSERT_EQ(33.5, m_dpowerDevice->temperature());
}

TEST_F(TestDPowerDevice, voltage)
{
    ASSERT_EQ(20.7, m_fakeInterface->voltage());
    ASSERT_EQ(20.7, m_dpowerDevice->voltage());
}

TEST_F(TestDPowerDevice, chargeCycles)
{
    ASSERT_EQ(4, m_fakeInterface->chargeCycles());
    ASSERT_EQ(4, m_dpowerDevice->chargeCycles());
}

TEST_F(TestDPowerDevice, timeToEmpty)
{
    ASSERT_EQ(12345678, m_fakeInterface->timeToEmpty());
    ASSERT_EQ(12345678, m_dpowerDevice->timeToEmpty());
}

TEST_F(TestDPowerDevice, timeToFull)
{
    ASSERT_EQ(12345678, m_fakeInterface->timeToFull());
    ASSERT_EQ(12345678, m_dpowerDevice->timeToFull());
}

TEST_F(TestDPowerDevice, iconName)
{
    ASSERT_EQ("full", m_fakeInterface->iconName());
    ASSERT_EQ("full", m_dpowerDevice->iconName());
}

TEST_F(TestDPowerDevice, model)
{
    ASSERT_EQ("online", m_fakeInterface->model());
    ASSERT_EQ("online", m_dpowerDevice->model());
}

TEST_F(TestDPowerDevice, nativePath)
{
    ASSERT_EQ("path", m_fakeInterface->nativePath());
    ASSERT_EQ("path", m_dpowerDevice->nativePath());
}

TEST_F(TestDPowerDevice, serial)
{
    ASSERT_EQ("ok", m_fakeInterface->serial());
    ASSERT_EQ("ok", m_dpowerDevice->serial());
}

TEST_F(TestDPowerDevice, vendor)
{
    ASSERT_EQ("ok", m_fakeInterface->vendor());
    ASSERT_EQ("ok", m_dpowerDevice->vendor());
}

TEST_F(TestDPowerDevice, batteryLevel)
{
    ASSERT_EQ(2, m_fakeInterface->batteryLevel());
    ASSERT_EQ(2, m_dpowerDevice->batteryLevel());
}

TEST_F(TestDPowerDevice, state)
{
    ASSERT_EQ(2, m_fakeInterface->state());
    ASSERT_EQ(2, m_dpowerDevice->state());
}

TEST_F(TestDPowerDevice, technology)
{
    ASSERT_EQ(2, m_fakeInterface->technology());
    ASSERT_EQ(2, m_dpowerDevice->technology());
}

TEST_F(TestDPowerDevice, type)
{
    ASSERT_EQ(2, m_fakeInterface->type());
    ASSERT_EQ(2, m_dpowerDevice->type());
}

TEST_F(TestDPowerDevice, warningLevel)
{
    ASSERT_EQ(2, m_fakeInterface->warningLevel());
    ASSERT_EQ(2, m_dpowerDevice->warningLevel());
}

TEST_F(TestDPowerDevice, updateTime)
{
    ASSERT_EQ(166443275, m_fakeInterface->updateTime());
    qDebug() << m_dpowerDevice->updateTime();
    ASSERT_EQ(166443275, m_dpowerDevice->updateTime().toSecsSinceEpoch());
}

TEST_F(TestDPowerDevice, deviceName)
{
    ASSERT_EQ("battery", m_fakeInterface->deviceName());
    ASSERT_EQ("display", m_dpowerDevice->deviceName());
}

TEST_F(TestDPowerDevice, history)
{
    auto reval = m_fakeInterface->GetHistory("type", 100, 100);
    ASSERT_EQ(100, reval[0].time);
    ASSERT_EQ(100, reval[0].value);
    ASSERT_EQ(100, reval[0].state);
    ASSERT_EQ(200, reval[1].value);
    auto reval1 = m_dpowerDevice->history("type", 100, 100).value();
    ASSERT_EQ(100, reval1[0].time);
    ASSERT_EQ(100, reval1[0].value);
    ASSERT_EQ(100, reval1[0].state);
    ASSERT_EQ(200, reval1[1].value);
}

TEST_F(TestDPowerDevice, statistic)
{
    auto reval = m_fakeInterface->GetStatistics("type");
    ASSERT_EQ(100, reval[0].value);
    ASSERT_EQ(100, reval[0].accuracy);
    ASSERT_EQ(200, reval[1].value);
    ASSERT_EQ(200, reval[1].accuracy);
    auto reval1 = m_dpowerDevice->statistics("type").value();
    ASSERT_EQ(100, reval1[0].value);
    ASSERT_EQ(100, reval1[0].accuracy);
    ASSERT_EQ(200, reval1[1].value);
    ASSERT_EQ(200, reval1[1].accuracy);
}
