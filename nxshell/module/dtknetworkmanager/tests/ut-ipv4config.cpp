// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "dipv4config.h"
#include "fakeDBus/ipv4configservice.h"
#include "fakeDBus/networkmanagerservice.h"
#include <QDebug>

DNETWORKMANAGER_USE_NAMESPACE

class TestDIPv4Config : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_nm = new FakeNetworkManagerService();
        m_fakeService = new FakeIPv4ConfigService();
        m_ic = new DIPv4Config(2);
    }

    static void TearDownTestCase()
    {
        delete m_ic;
        delete m_fakeService;
        delete m_nm;
    }

    void SetUp() override {}
    void TearDown() override {}

    static inline DIPv4Config *m_ic{nullptr};
    static inline FakeIPv4ConfigService *m_fakeService{nullptr};
    static inline FakeNetworkManagerService *m_nm{nullptr};
};

TEST_F(TestDIPv4Config, addressData)
{
    EXPECT_EQ("192.168.0.100", m_ic->addressData()[0].find("address")->toString());
}

TEST_F(TestDIPv4Config, nameserverData)
{
    EXPECT_EQ("1.1.1.1", m_ic->nameserverData()[0].find("address")->toString());
}

TEST_F(TestDIPv4Config, gateway)
{
    EXPECT_EQ("192.168.0.1", m_ic->gateway().toString());
}
