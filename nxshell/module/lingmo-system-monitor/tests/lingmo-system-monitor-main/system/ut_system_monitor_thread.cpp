// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/system_monitor_thread.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace core::system;

class UT_SystemMonitorThread: public ::testing::Test
{
public:
    UT_SystemMonitorThread() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new SystemMonitorThread();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    SystemMonitorThread *m_tester;
};

TEST_F(UT_SystemMonitorThread, initTest)
{
}

TEST_F(UT_SystemMonitorThread, test_systemMonitorInstance)
{
    EXPECT_TRUE(m_tester->systemMonitorInstance() != nullptr);
}
