// SPDX-FileCopyrightText: 2017 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
#include "dtkcore_global.h"
#include <gtest/gtest.h>
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
#include "dtimedloop.h"
DCORE_USE_NAMESPACE
#endif

// 有返回值的 lambda 表达式、函数里面使用 ASSERT_XXX
// 总之，不管有没有返回值，用它就对了
#ifndef HAVE_FUN
#define HAVE_FUN(X) [&](){X;}();
#endif

class ut_DUtil : public testing::Test
{
protected:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp();
    virtual void TearDown();
};
