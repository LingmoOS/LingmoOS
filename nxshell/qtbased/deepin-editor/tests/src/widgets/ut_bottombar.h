// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEST_BOTTOMBAR_H
#define TEST_BOTTOMBAR_H

#include "../../src/widgets/bottombar.h"

#include <gtest/gtest.h>

class TestBottomBar : public testing::Test
{
protected:
    void SetUp()
    {
        bottomBar = new BottomBar();
    }
    void TearDown()
    {
        delete bottomBar;
    }

    BottomBar *bottomBar = nullptr;
};

#endif // TEST_BOTTOMBAR_H
