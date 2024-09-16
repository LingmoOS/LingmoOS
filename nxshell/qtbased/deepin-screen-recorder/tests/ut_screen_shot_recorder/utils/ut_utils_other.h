// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>

using namespace testing;

class Utils_OtherTest:public testing::Test{

public:
    virtual void SetUp() override{
        std::cout << "start Utils_OtherTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end Utils_OtherTest" << std::endl;
    }
};

TEST_F(Utils_OtherTest, Utils_OtherTest1)
{

}
