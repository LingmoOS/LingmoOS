// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "dsystemproxy.h"
#include "fakeDBus/systemproxyservice.h"

DNETWORKMANAGER_USE_NAMESPACE

class TestDSystemProxy : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_fakeService = new FakeSystemProxyService();
        m_sp = new DSystemProxy();
    }

    static void TearDownTestCase()
    {
        delete m_sp;
        delete m_fakeService;
    }

    void SetUp() override {}
    void TearDown() override {}

    static inline DSystemProxy *m_sp{nullptr};
    static inline FakeSystemProxyService *m_fakeService{nullptr};
};

TEST_F(TestDSystemProxy, getAutoProxy)
{
    m_sp->autoProxy();
    EXPECT_EQ(true, m_fakeService->m_getAutoProxyTrigger);
}

TEST_F(TestDSystemProxy, getProxy)
{
    m_sp->proxy({});
    EXPECT_EQ(true, m_fakeService->m_getProxyTrigger);
}

TEST_F(TestDSystemProxy, getProxyIgnoreHosts)
{
    m_sp->proxyIgnoreHosts();
    EXPECT_EQ(true, m_fakeService->m_getProxyIgnoreHostsTrigger);
}

TEST_F(TestDSystemProxy, getProxyMethod)
{
    m_sp->proxyMethod();
    EXPECT_EQ(true, m_fakeService->m_getProxyMethodTrigger);
}

TEST_F(TestDSystemProxy, setAutoProxy)
{
    QUrl tmp{"https://proxyserver.com/proxy.pac"};
    m_sp->setAutoProxy(tmp);
    EXPECT_EQ(true, m_fakeService->m_setAutoProxyTrigger);
}

TEST_F(TestDSystemProxy, setProxy)
{
    m_sp->setProxy({}, {});
    EXPECT_EQ(true, m_fakeService->m_setProxyTrigger);
}

TEST_F(TestDSystemProxy, setProxyIgnoreHosts)
{
    m_sp->setProxyIgnoreHosts({});
    EXPECT_EQ(true, m_fakeService->m_setProxyIgnoreHostsTrigger);
}

TEST_F(TestDSystemProxy, setProxyMethod)
{
    m_sp->setProxyMethod(SystemProxyMethod::Auto);
    EXPECT_EQ(true, m_fakeService->m_setProxyMethodTrigger);
}
