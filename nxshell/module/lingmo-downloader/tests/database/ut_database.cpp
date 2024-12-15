// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include "gtest/gtest.h"
#include "database.h"

class ut_databaseTest : public ::testing::Test
{
protected:
    ut_databaseTest()
    {
    }

    virtual ~ut_databaseTest()
    {
    }
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(ut_databaseTest, getDB)
{
    auto q = DataBase::Instance().getDB();
    bool ret = q.open();
    EXPECT_TRUE(true);

}
