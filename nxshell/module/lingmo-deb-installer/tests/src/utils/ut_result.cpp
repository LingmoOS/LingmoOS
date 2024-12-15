// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "../src/deb-installer/utils/result.h"
#include <QString>
typedef Result<QString> TestResult;

TEST(Result_Test, Result_UT_001)
{
    TestResult t = TestResult::err("test");
}

TEST(Result_Test, Result_UT_002)
{
    TestResult t = TestResult::err("test");
    t.err("test");
    t.unwrap();
}
TEST(Result_Test, Result_UT_003)
{
    Result<QString> t(true, "QString");
    t.is_ok();
}
