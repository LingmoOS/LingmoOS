// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "daccesspoint.h"
#include "fakeDBus/accesspointservice.h"
#include "fakeDBus/networkmanagerservice.h"

DNETWORKMANAGER_USE_NAMESPACE

class TestDAccessPoint : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_nm = new FakeNetworkManagerService();
        m_fakeService = new FakeAccessPointService();
        m_ap = new DAccessPoint(15);
    }

    static void TearDownTestCase()
    {
        delete m_ap;
        delete m_fakeService;
        delete m_nm;
    }

    void SetUp() override {}
    void TearDown() override {}

    static inline DAccessPoint *m_ap{nullptr};
    static inline FakeAccessPointService *m_fakeService{nullptr};
    static inline FakeNetworkManagerService *m_nm{nullptr};
};

TEST_F(TestDAccessPoint, SSID)
{
    EXPECT_EQ("uniontech", m_ap->SSID());
}

TEST_F(TestDAccessPoint, strength)
{
    EXPECT_EQ(static_cast<quint8>(20), m_ap->strength());
}

TEST_F(TestDAccessPoint, flags)
{
    const auto flag = m_ap->flags();
    EXPECT_EQ(true, flag.testFlag(NM80211ApFlags::Privacy));
    EXPECT_EQ(true, flag.testFlag(NM80211ApFlags::WPS));
}

TEST_F(TestDAccessPoint, frequency)
{
    EXPECT_EQ(3152, m_ap->frequency());
}

TEST_F(TestDAccessPoint, rsnFlags)
{
    const auto rsn = m_ap->rsnFlags();
    EXPECT_EQ(true, rsn.testFlag(NM80211ApSecurityFlags::PairWEP40));
    EXPECT_EQ(true, rsn.testFlag(NM80211ApSecurityFlags::PairWEP104));
}

TEST_F(TestDAccessPoint, wpaFlags)
{
    const auto wpa = m_ap->wpaFlags();
    EXPECT_EQ(true, wpa.testFlag(NM80211ApSecurityFlags::PairWEP40));
    EXPECT_EQ(true, wpa.testFlag(NM80211ApSecurityFlags::PairWEP104));
}
