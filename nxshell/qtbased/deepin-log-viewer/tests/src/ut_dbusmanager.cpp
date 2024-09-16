// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "../application/dbusmanager.h"
#include <stub.h>

TEST(DBusManager_Constructor_UT, DBusManager_Constructor_UT_001)
{
    DBusManager *p = new DBusManager(nullptr);
    EXPECT_NE(p, nullptr);
    delete p;
}
TEST(DBusManager_GetSystemInfo_UT, DBusManager_GetSystemInfo_UT_001)
{
    DBusManager *p = new DBusManager(nullptr);
    EXPECT_NE(p, nullptr);
    bool rs = p->isSEOpen();
    EXPECT_EQ(rs, true);
    delete p;
}
