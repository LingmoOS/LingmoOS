// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "obextransferservice.h"
#include <gtest/gtest.h>
#include "dbluetoothobextransfer.h"
#include "dbluetoothtypes.h"
#include <QFileInfo>

DBLUETOOTH_USE_NAMESPACE

class TestObexTransfer : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_ot = new DObexTransfer(ObexSessionInfo{ObexSessionType::Client, 0}, 0);
        m_fakeService = new FakeObexTransferService();
    }

    static void TearDownTestCase()
    {
        delete m_ot;
        delete m_fakeService;
    }

    void SetUp() override {}
    void TearDown() override {}

private:
    static inline DObexTransfer *m_ot{nullptr};
    static inline FakeObexTransferService *m_fakeService{nullptr};
};

TEST_F(TestObexTransfer, status)
{
    EXPECT_EQ(m_ot->status(), DObexTransfer::TransferStatus::Queued);
}

TEST_F(TestObexTransfer, session)
{
    ObexSessionInfo tmp{ObexSessionType::Client, 0};
    EXPECT_EQ(m_ot->session(), tmp);
}

TEST_F(TestObexTransfer, type)
{
    EXPECT_EQ(m_ot->type(), "text");
}

TEST_F(TestObexTransfer, name)
{
    EXPECT_EQ(m_ot->name(), "transfer1");
}

TEST_F(TestObexTransfer, size)
{
    EXPECT_EQ(m_ot->size(), 1024);
}

TEST_F(TestObexTransfer, transferred)
{
    EXPECT_EQ(m_ot->transferred(), 512);
}

TEST_F(TestObexTransfer, filename)
{
    EXPECT_EQ(m_ot->filename(), QFileInfo("1.txt"));
}

TEST_F(TestObexTransfer, Cancel)
{
    EXPECT_FALSE(m_fakeService->m_cancel);
    m_ot->cancel();
    EXPECT_TRUE(m_fakeService->m_cancel);
}
