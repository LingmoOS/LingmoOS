// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "dexpected.h"

DCORE_USE_NAMESPACE

TEST(ut_DExpected, unexpected)
{
    DExpected<void> exp_void { DUnexpected { emplace_tag::USE_EMPLACE, 404, "Not Found"} };

    EXPECT_FALSE(exp_void.hasValue());
    EXPECT_EQ(exp_void.error().getErrorCode(), 404);
    EXPECT_EQ(exp_void.error().getErrorMessage(), "Not Found");
}

TEST(ut_DExpected, expected)
{
    DExpected<void> exp_void {};
    EXPECT_TRUE(exp_void.hasValue()); // void

    DExpected<int> exp_int {200};
    EXPECT_TRUE(exp_int.hasValue());
    EXPECT_EQ(exp_int.value(), 200);
}

TEST(ut_DError, defaultError)
{
    DError error;
    EXPECT_EQ(error.getErrorCode(), -1);
    EXPECT_EQ(error.getErrorMessage(), QString());
}

TEST(ut_DError, setError)
{
    DError error(404, "Not Found");
    EXPECT_EQ(error.getErrorCode(), 404);
    EXPECT_EQ(error.getErrorMessage(), "Not Found");

    error.setErrorCode(502);
    error.setErrorMessage("Bad Gateway");
    EXPECT_EQ(error.getErrorCode(), 502);
    EXPECT_EQ(error.getErrorMessage(), "Bad Gateway");
}

TEST(ut_DError, operator)
{
    DError error(404, "Not Found");
    DError copy = error;
    EXPECT_EQ(copy, error);

    copy.setErrorCode(502);
    EXPECT_NE(copy, error);

    qDebug() << error; // operator <<
}
