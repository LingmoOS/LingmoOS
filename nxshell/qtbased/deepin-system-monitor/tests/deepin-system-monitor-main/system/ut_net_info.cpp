// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/net_info.h"
#include <QDebug>

//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace core::system;

class UT_NetInfo: public ::testing::Test
{
public:
    UT_NetInfo() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new NetInfo();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    NetInfo *m_tester;
};

TEST_F(UT_NetInfo, initTest)
{
}

TEST_F(UT_NetInfo, test_recvBps)
{
    m_tester->resdNetInfo();
    EXPECT_NE(m_tester->recvBps(), 0);
}

TEST_F(UT_NetInfo, test_sentBps)
{
    m_tester->resdNetInfo();
    EXPECT_NE(m_tester->sentBps(), 0);
}

TEST_F(UT_NetInfo, test_totalRecvBytes)
{
    m_tester->resdNetInfo();
    sleep(1);
    m_tester->resdNetInfo();
}

TEST_F(UT_NetInfo, test_totalSentBytes)
{
    m_tester->resdNetInfo();
    sleep(1);
    m_tester->resdNetInfo();
}

TEST_F(UT_NetInfo, test_resdNetInfo)
{
    m_tester->resdNetInfo();
    sleep(1);
    m_tester->resdNetInfo();
}

