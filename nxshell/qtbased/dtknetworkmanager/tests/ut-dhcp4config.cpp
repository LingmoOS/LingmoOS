// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "ddhcp4config.h"
#include "fakeDBus/dhcp4configservice.h"
#include "fakeDBus/networkmanagerservice.h"
#include <QDebug>

DNETWORKMANAGER_USE_NAMESPACE

class TestDDHCP4Config : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_nm = new FakeNetworkManagerService();
        m_fakeService = new FakeDHCP4ConfigService();
        m_dc = new DDHCP4Config(2);
    }

    static void TearDownTestCase()
    {
        delete m_dc;
        delete m_fakeService;
        delete m_nm;
    }

    void SetUp() override {}
    void TearDown() override {}

    static inline DDHCP4Config *m_dc{nullptr};
    static inline FakeDHCP4ConfigService *m_fakeService{nullptr};
    static inline FakeNetworkManagerService *m_nm{nullptr};
};

TEST_F(TestDDHCP4Config, options)
{
    EXPECT_EQ("1.1.1.1", m_dc->options().find("broadcast_address")->toString());
}
