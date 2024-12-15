// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "common/thread_manager.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

//qt
#include <sys/time.h>
#include <QApplication>
#include <QDateTime>

using namespace common;
using namespace core;
/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

class UT_ThreadManager : public ::testing::Test
{
public:
    UT_ThreadManager() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ThreadManager();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    ThreadManager *m_tester;
};

TEST_F(UT_ThreadManager, initTest)
{

}

TEST_F(UT_ThreadManager, test_attach_01)
{
    EXPECT_EQ(m_tester->m_threadDB.isEmpty(),true);
}
