// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "managerservice.h"
#include <gtest/gtest.h>
#include "dbluetoothmanager.h"
#include "dbluetoothagent.h"

DBLUETOOTH_USE_NAMESPACE

class testAgent : public DAgent
{
public:
    explicit testAgent(QObject *parent = nullptr)
        : DAgent(parent)
    {
    }

    ~testAgent() = default;
    QDBusObjectPath agentPath() const override { return QDBusObjectPath("/org/deepin/agent"); }
};

class TestDManager : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_ma = new DManager();
        m_fakeService = new FakeAgentManagerService();
        m_agent = QSharedPointer<testAgent>(new testAgent);
    }

    static void TearDownTestCase()
    {
        delete m_ma;
        delete m_fakeService;
        m_agent->deleteLater();
    }

    void SetUp() override {}
    void TearDown() override {}

private:
    static inline DManager *m_ma{nullptr};
    static inline FakeAgentManagerService *m_fakeService{nullptr};
    static inline QSharedPointer<testAgent> m_agent{nullptr};
};

TEST_F(TestDManager, registerAgent)
{
    EXPECT_FALSE(m_fakeService->m_registerAgent);
    auto reply = m_ma->registerAgent(m_agent);
    EXPECT_TRUE(m_fakeService->m_registerAgent);
}

TEST_F(TestDManager, requestDefaultAgent)
{
    EXPECT_FALSE(m_fakeService->m_requestDefaultAgent);
    m_ma->requestDefaultAgent(m_agent);
    EXPECT_TRUE(m_fakeService->m_requestDefaultAgent);
}

TEST_F(TestDManager, unregisterAgent)
{
    EXPECT_FALSE(m_fakeService->m_unregisterAgent);
    m_ma->unregisterAgent(m_agent);
    EXPECT_TRUE(m_fakeService->m_unregisterAgent);
}
