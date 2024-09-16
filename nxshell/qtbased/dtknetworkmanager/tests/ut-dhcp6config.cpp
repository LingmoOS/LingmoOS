// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "ddhcp6config.h"
#include "fakeDBus/dhcp6configservice.h"
#include "fakeDBus/networkmanagerservice.h"
#include <QDebug>

DNETWORKMANAGER_USE_NAMESPACE

class TestDDHCP6Config : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_nm = new FakeNetworkManagerService();
        m_fakeService = new FakeDHCP6ConfigService();
        m_dc = new DDHCP6Config(2);
    }

    static void TearDownTestCase()
    {
        delete m_dc;
        delete m_fakeService;
        delete m_nm;
    }

    void SetUp() override {}
    void TearDown() override {}

    static inline DDHCP6Config *m_dc{nullptr};
    static inline FakeDHCP6ConfigService *m_fakeService{nullptr};
    static inline FakeNetworkManagerService *m_nm{nullptr};
};

TEST_F(TestDDHCP6Config, options)
{
    EXPECT_EQ("::1/128", m_dc->options().find("ip_address")->toString());
}
