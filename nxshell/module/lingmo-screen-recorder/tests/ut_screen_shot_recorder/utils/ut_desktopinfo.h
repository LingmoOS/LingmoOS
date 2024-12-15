// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include "../../src/utils/desktopinfo.h"


using namespace testing;

class DesktopInfoTest:public testing::Test{

public:
    DesktopInfo deskInfo;
    virtual void SetUp() override{
        std::cout << "start DesktopInfoTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end DesktopInfoTest" << std::endl;
    }
};

TEST_F(DesktopInfoTest, waylandDectected)
{
    EXPECT_FALSE(deskInfo.waylandDectected());
}
