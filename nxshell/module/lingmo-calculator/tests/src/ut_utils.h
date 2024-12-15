// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_UTILS_H
#define UT_UTILS_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>

class Ut_Utils : public ::testing::Test
{
public:
    Ut_Utils();
};

#endif // UT_UTILS_H
