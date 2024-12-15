// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "common/time_period.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace common;
using namespace core;
/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

class UT_TimePeriod : public ::testing::Test
{
public:
    UT_TimePeriod() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new TimePeriod();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    TimePeriod *m_tester;
};

TEST_F(UT_TimePeriod, initTest)
{

}

TEST_F(UT_TimePeriod, test_registerMetaType_01)
{
    EXPECT_TRUE(qRegisterMetaType<TimePeriod>("TimePeriod"));
}

TEST_F(UT_TimePeriod, test_registerMetaType_02)
{
    m_tester->registerMetaType();
}

TEST_F(UT_TimePeriod, test_ticks_01)
{
    size_t TimePeriod = m_tester->m_period;
    EXPECT_EQ(TimePeriod,m_tester->m_period);
}

TEST_F(UT_TimePeriod, test_ticks_02)
{
    m_tester->m_interval.tv_sec= 0;
    m_tester->m_interval.tv_usec = 0;
    EXPECT_EQ(m_tester->ticks(), 0);
}

TEST_F(UT_TimePeriod, test_ticks_03)
{
    m_tester->m_interval.tv_sec= 1;
    m_tester->m_interval.tv_usec = 0;
    m_tester->m_period = TimePeriod::time_period_t::k5Min;
    m_tester->ticks();
}

TEST_F(UT_TimePeriod, test_operator_01)
{
    const TimePeriod rhs;
    m_tester->operator==(rhs);
}

TEST_F(UT_TimePeriod, test_operator_02)
{
    const TimePeriod rhs;
    m_tester->operator!=(rhs);
}
