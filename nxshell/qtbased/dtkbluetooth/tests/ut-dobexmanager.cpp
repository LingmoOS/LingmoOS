// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbluetoothobexsession.h"
#include "obexmanagerservice.h"
#include <gtest/gtest.h>
#include "dbluetoothobexmanager.h"
#include "dbluetoothobexagent.h"

DBLUETOOTH_USE_NAMESPACE

class testAgent : public DObexAgent
{
    explicit testAgent(QObject *parent = nullptr)
        : DObexAgent(parent)
    {
    }
    ~testAgent() = default;
    QDBusObjectPath agentPath() const override { return QDBusObjectPath("/org/deepin/obexAgent"); }
};

class TestDObexAgentManager : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_om = new DObexManager();
        m_fakeAgentService = new FakeObexAgentManagerService();
        m_agent = QSharedPointer<testAgent>(new testAgent);
    }

    static void TearDownTestCase()
    {
        m_agent->deleteLater();
        delete m_om;
        delete m_fakeAgentService;
    }

    void SetUp() override {}
    void TearDown() override {}

private:
    static inline DObexManager *m_om{nullptr};
    static inline FakeObexAgentManagerService *m_fakeAgentService{nullptr};
    static inline QSharedPointer<testAgent> m_agent;
};

class TestDObexClient : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_om = new DObexManager();
        m_fakeClientService = new FakeObexClientService();
        m_agent = QSharedPointer<testAgent>(new testAgent);
    }

    static void TearDownTestCase()
    {
        m_agent->deleteLater();
        delete m_om;
        delete m_fakeClientService;
    }

    void SetUp() override {}
    void TearDown() override {}

private:
    static inline DObexManager *m_om{nullptr};
    static inline FakeObexClientService *m_fakeClientService{nullptr};
    static inline QSharedPointer<testAgent> m_agent;
};

TEST_F(TestDObexAgentManager, registerAgent)
{
    EXPECT_FALSE(m_fakeAgentService->m_registerAgent);
    auto reply = m_om->registerAgent(m_agent);
    if (!reply)
        qDebug() << reply.error();
    EXPECT_TRUE(m_fakeAgentService->m_registerAgent);
}

TEST_F(TestDObexAgentManager, unregisterAgent)
{
    EXPECT_FALSE(m_fakeAgentService->m_unregisterAgent);
    m_om->unregisterAgent(m_agent);
    EXPECT_TRUE(m_fakeAgentService->m_unregisterAgent);
}

TEST_F(TestDObexClient, createSession)
{
    EXPECT_FALSE(m_fakeClientService->m_createSession);
    auto reply = m_om->createSession("", {});
    if (!reply)
        qDebug() << reply.error();
    EXPECT_TRUE(m_fakeClientService->m_createSession);
}

TEST_F(TestDObexClient, removeSession)
{
    EXPECT_FALSE(m_fakeClientService->m_removeSession);
    auto session = QSharedPointer<DObexSession>(new DObexSession(ObexSessionInfo{}));
    m_om->removeSession(session);
    EXPECT_TRUE(m_fakeClientService->m_createSession);
}
