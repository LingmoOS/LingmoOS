// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DebugTimeManager.h"
#include <stub.h>
#include <gtest/gtest.h>


TEST(UT_DebugTimeManager_clear, UT_DebugTimeManager_clear_001)
{
    DebugTimeManager *Dtime = new DebugTimeManager();
    ASSERT_TRUE(Dtime);
    Dtime->clear();
    delete Dtime;
}
