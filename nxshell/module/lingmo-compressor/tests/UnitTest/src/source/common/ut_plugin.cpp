// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugin.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/

/*******************************单元测试************************************/
// 测试Plugin
class UT_Plugin : public ::testing::Test
{
public:
    UT_Plugin(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new Plugin(nullptr);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    Plugin *m_tester;
};

TEST_F(UT_Plugin, initTest)
{

}

TEST_F(UT_Plugin, test_priority)
{
    EXPECT_EQ(m_tester->priority(), 0);
}

TEST_F(UT_Plugin, test_isEnabled)
{
    EXPECT_EQ(m_tester->isEnabled(), m_tester->m_enabled);
}

TEST_F(UT_Plugin, test_setEnabled)
{
    m_tester->m_enabled = false;
    m_tester->setEnabled(true);
    EXPECT_EQ(m_tester->m_enabled, true);
}

TEST_F(UT_Plugin, test_isReadWrite)
{
    EXPECT_EQ(m_tester->isReadWrite(), false);
}

TEST_F(UT_Plugin, test_readOnlyExecutables)
{
    EXPECT_EQ(m_tester->isReadWrite(), false);
}
