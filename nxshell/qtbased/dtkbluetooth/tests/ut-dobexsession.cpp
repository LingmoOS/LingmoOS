// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "obexsessionservice.h"
#include <gtest/gtest.h>
#include "dbluetoothobexsession.h"

DBLUETOOTH_USE_NAMESPACE

class TestObexSession : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_os = new DObexSession(ObexSessionInfo{ObexSessionType::Client, 0});
        m_fakeService = new FakeObexSessionService();
    }

    static void TearDownTestCase()
    {
        delete m_os;
        delete m_fakeService;
    }

    void SetUp() override {}
    void TearDown() override {}

private:
    static inline DObexSession *m_os{nullptr};
    static inline FakeObexSessionService *m_fakeService{nullptr};
};

TEST_F(TestObexSession, source)
{
    EXPECT_EQ("AA:AA:AA:AA:AA:AA", m_os->source());
}

TEST_F(TestObexSession, destination)
{
    EXPECT_EQ("BB:BB:BB:BB:BB:BB", m_os->destination());
}

TEST_F(TestObexSession, target)
{
    EXPECT_EQ(QBluetoothUuid(QString("XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX")), m_os->target());
}

TEST_F(TestObexSession, root)
{
    EXPECT_EQ(QString("/home/test/.cache/obex"), m_os->root().absolutePath());
}

TEST_F(TestObexSession, GetCapabilities)
{
    EXPECT_FALSE(m_fakeService->m_getCap);
    m_os->capabilities();
    EXPECT_TRUE(m_fakeService->m_getCap);
}
