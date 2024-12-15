// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "processinfomanager.h"
#undef private

#include <gtest/gtest.h>

class UT_ProcessInfoManager : public testing::Test
{

};

TEST_F(UT_ProcessInfoManager, coverageTest)
{
    ProcessInfoManager manager;
    manager.scanChromeTabs();
}
