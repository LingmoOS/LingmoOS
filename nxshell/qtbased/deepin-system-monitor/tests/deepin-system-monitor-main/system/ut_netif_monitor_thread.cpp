// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/netif_monitor_thread.h"
#include "system/netif_monitor.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace core::system;

class UT_NetifMonitorThread: public ::testing::Test
{
public:
    UT_NetifMonitorThread() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new NetifMonitorThread();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    NetifMonitorThread *m_tester;
};

TEST_F(UT_NetifMonitorThread, initTest)
{
}

TEST_F(UT_NetifMonitorThread, test_netifJobInstance)
{
    EXPECT_TRUE(m_tester->netifJobInstance() != nullptr);
}

