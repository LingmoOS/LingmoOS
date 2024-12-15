// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "common/perf.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

//qt
#include <sys/time.h>
#include <QApplication>
#include <QDateTime>

/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

class UT_Perf : public ::testing::Test
{
public:
    UT_Perf() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new DebugTimeManager();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    DebugTimeManager *m_tester;
};

TEST_F(UT_Perf, initTest)
{

}

TEST_F(UT_Perf, test_clear_01)
{
    m_tester->m_MapPoint.clear();
    EXPECT_EQ(m_tester->m_MapPoint.isEmpty(),true);
}

TEST_F(UT_Perf, test_beginPointLinux_01)
{
    m_tester->beginPointLinux("");
}

TEST_F(UT_Perf, test_endPointLinux_01)
{
    m_tester->endPointLinux("");
}

