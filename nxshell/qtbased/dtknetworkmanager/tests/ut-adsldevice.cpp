// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "dadsldevice.h"
#include "fakeDBus/adsldeviceservice.h"
#include "fakeDBus/networkmanagerservice.h"

DNETWORKMANAGER_USE_NAMESPACE

class TestDAdslDevice : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_nm = new FakeNetworkManagerService();
        m_fakeService = new FakeAdslDeviceService();
        m_ad = new DAdslDevice(2);
    }

    static void TearDownTestCase()
    {
        delete m_ad;
        delete m_fakeService;
        delete m_nm;
    }

    void SetUp() override {}
    void TearDown() override {}

    static inline DAdslDevice *m_ad{nullptr};
    static inline FakeAdslDeviceService *m_fakeService{nullptr};
    static inline FakeNetworkManagerService *m_nm{nullptr};
};

TEST_F(TestDAdslDevice, availableConnections)
{
    EXPECT_EQ(2, m_ad->availableConnections()[0]);
}

TEST_F(TestDAdslDevice, autoconnect)
{
    EXPECT_EQ(true, m_ad->autoconnect());
}

TEST_F(TestDAdslDevice, managed)
{
    EXPECT_EQ(true, m_ad->managed());
}

TEST_F(TestDAdslDevice, activeConnection)
{
    EXPECT_EQ(1, m_ad->activeConnection());
}

TEST_F(TestDAdslDevice, DHCP4Config)
{
    EXPECT_EQ(1, m_ad->DHCP4Config());
}

TEST_F(TestDAdslDevice, DHCP6Config)
{
    EXPECT_EQ(1, m_ad->DHCP6Config());
}

TEST_F(TestDAdslDevice, IPv4Config)
{
    EXPECT_EQ(1, m_ad->IPv4Config());
}

TEST_F(TestDAdslDevice, IPv6Config)
{
    EXPECT_EQ(1, m_ad->IPv6Config());
}

TEST_F(TestDAdslDevice, driver)
{
    EXPECT_EQ("e1000e", m_ad->driver());
}

TEST_F(TestDAdslDevice, interface)
{
    EXPECT_EQ("eno1", m_ad->interface());
}

TEST_F(TestDAdslDevice, udi)
{
    EXPECT_EQ("/sys/devices/pci0000:00/0000:00:1f.6/net/eno1", m_ad->udi());
}

TEST_F(TestDAdslDevice, deviceType)
{
    EXPECT_EQ(NMDeviceType::Ethernet, m_ad->deviceType());
}

TEST_F(TestDAdslDevice, interfaceFlags)
{
    EXPECT_EQ(true, m_ad->interfaceFlags().testFlag(NMDeviceInterfaceFlags::UP));
    EXPECT_EQ(true, m_ad->interfaceFlags().testFlag(NMDeviceInterfaceFlags::LowerUP));
}

TEST_F(TestDAdslDevice, state)
{
    EXPECT_EQ(NMDeviceState::Activated, m_ad->deviceState());
}

TEST_F(TestDAdslDevice, disconnect)
{
    m_ad->disconnect();
    EXPECT_EQ(true, m_fakeService->m_disconnectTrigger);
}

TEST_F(TestDAdslDevice, refreshRateMs)
{
    EXPECT_EQ(10, m_ad->refreshRateMs());
    m_ad->setRefreshRateMs(20);
    EXPECT_EQ(20, m_ad->refreshRateMs());
}

TEST_F(TestDAdslDevice, rxBytes)
{
    EXPECT_EQ(3718, m_ad->rxBytes());
}

TEST_F(TestDAdslDevice, txBytes)
{
    EXPECT_EQ(910, m_ad->txBytes());
}

TEST_F(TestDAdslDevice, carrier)
{
    EXPECT_EQ(true, m_ad->carrier());
}
