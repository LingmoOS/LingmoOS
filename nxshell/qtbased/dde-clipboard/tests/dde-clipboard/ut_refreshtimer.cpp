// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "refreshtimer.h"

class TstRefreshTimer : public testing::Test
{
public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(TstRefreshTimer, coverageTest)
{
    RefreshTimer::instance();
}
