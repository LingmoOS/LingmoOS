// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "dappproxy.h"
#include "fakeDBus/appproxyservice.h"

DNETWORKMANAGER_USE_NAMESPACE

class TestDAppProxy : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_fakeService = new FakeAppProxyService();
        m_ap = new DAppProxy();
    }

    static void TearDownTestCase()
    {
        delete m_ap;
        delete m_fakeService;
    }

    void SetUp() override {}
    void TearDown() override {}

    static inline DAppProxy *m_ap{nullptr};
    static inline FakeAppProxyService *m_fakeService{nullptr};
};

TEST_F(TestDAppProxy, IP)
{
    EXPECT_EQ("127.0.0.1", m_ap->IP().toString());
}

TEST_F(TestDAppProxy, password)
{
    EXPECT_EQ("123456789", m_ap->password());
}

TEST_F(TestDAppProxy, type)
{
    EXPECT_EQ(ProxyType::HTTP, m_ap->type());
}

TEST_F(TestDAppProxy, user)
{
    EXPECT_EQ("deepin", m_ap->user());
}

TEST_F(TestDAppProxy, port)
{
    EXPECT_EQ(1080, m_ap->port());
}

TEST_F(TestDAppProxy, Set)
{
    auto ret = m_ap->set({}, {}, {}, {}, {});
    if (!ret)
        qWarning() << ret.error();
    EXPECT_EQ(true, m_fakeService->m_SetTrigger);
}
