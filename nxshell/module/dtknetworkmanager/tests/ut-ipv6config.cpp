// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "dipv6config.h"
#include "fakeDBus/ipv6configservice.h"
#include "fakeDBus/networkmanagerservice.h"
#include "dnmutils.h"
#include <QDebug>

DNETWORKMANAGER_USE_NAMESPACE

class TestDIPv6Config : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_nm = new FakeNetworkManagerService();
        m_fakeService = new FakeIPv6ConfigService();
        m_ic = new DIPv6Config(2);
    }

    static void TearDownTestCase()
    {
        delete m_ic;
        delete m_fakeService;
        delete m_nm;
    }

    void SetUp() override {}
    void TearDown() override {}

    static inline DIPv6Config *m_ic{nullptr};
    static inline FakeIPv6ConfigService *m_fakeService{nullptr};
    static inline FakeNetworkManagerService *m_nm{nullptr};
};

TEST_F(TestDIPv6Config, addressData)
{
    EXPECT_EQ("::1", m_ic->addressData()[0].find("address")->toString());
}

TEST_F(TestDIPv6Config, nameservers)
{
    EXPECT_EQ("2001:4860:4860::8888", m_ic->nameservers()[0].toString());
}

TEST_F(TestDIPv6Config, gateway)
{
    EXPECT_EQ("fe80::1", m_ic->gateway().toString());
}
