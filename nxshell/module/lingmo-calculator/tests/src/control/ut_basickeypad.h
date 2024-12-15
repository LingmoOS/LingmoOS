// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_BASICKEYPAD_H
#define UT_BASICKEYPAD_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>

#include "../../src/dsettings.h"

class Ut_BasicKeypad : public ::testing::Test
{
public:
    Ut_BasicKeypad();
};

#endif // UT_BASICKEYPAD_H
