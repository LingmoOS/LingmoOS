// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "dwirelessdevice.h"
#include "fakeDBus/wirelessdeviceservice.h"
#include "fakeDBus/networkmanagerservice.h"
#include <QDebug>

DNETWORKMANAGER_USE_NAMESPACE

class TestDWirelessDevice : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_nm = new FakeNetworkManagerService();
        m_fakeService = new FakeWirelessDeviceService();
        m_wd = new DWirelessDevice(2);
    }

    static void TearDownTestCase()
    {
        delete m_wd;
        delete m_fakeService;
        delete m_nm;
    }

    void SetUp() override {}
    void TearDown() override {}

    static inline DWirelessDevice *m_wd{nullptr};
    static inline FakeWirelessDeviceService *m_fakeService{nullptr};
    static inline FakeNetworkManagerService *m_nm{nullptr};
};

TEST_F(TestDWirelessDevice, accessPoints)
{
    EXPECT_EQ(15, m_wd->accessPoints()[0]);
}

TEST_F(TestDWirelessDevice, HwAddress)
{
    EXPECT_EQ("127.0.0.1", m_wd->HwAddress());
}

TEST_F(TestDWirelessDevice, permHwAddress)
{
    EXPECT_EQ("127.0.0.1", m_wd->permHwAddress());
}

TEST_F(TestDWirelessDevice, mode)
{
    EXPECT_EQ(NM80211Mode::Infra, m_wd->mode());
}

TEST_F(TestDWirelessDevice, bitrate)
{
    EXPECT_EQ(200, m_wd->bitrate());
}

TEST_F(TestDWirelessDevice, activeAccessPoint)
{
    EXPECT_EQ(15, m_wd->activeAccessPoint());
}

TEST_F(TestDWirelessDevice, wirelessCapabilities)
{
    EXPECT_EQ(true, m_wd->wirelessCapabilities().testFlag(NMDeviceWiFiCapabilities::CipherWEP40));
    EXPECT_EQ(true, m_wd->wirelessCapabilities().testFlag(NMDeviceWiFiCapabilities::CipherWEP104));
}

TEST_F(TestDWirelessDevice,lastScan)
{
    EXPECT_EQ(1433143, m_wd->lastScan());
}

TEST_F(TestDWirelessDevice, requestScan)
{
    auto ret = m_wd->requestScan({});
    if (!ret)
        qWarning() << ret.error();
    EXPECT_EQ(true, m_fakeService->m_requestScanTrigger);
}

TEST_F(TestDWirelessDevice, getAllAccessPoints)
{
    auto ret = m_wd->getAllAccessPoints();
    if (!ret)
        qWarning() << ret.error();
    EXPECT_EQ(true, m_fakeService->m_getAllAccesspointTrigger);
}
