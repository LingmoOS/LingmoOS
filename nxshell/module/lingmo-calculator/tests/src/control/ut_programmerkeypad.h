// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_PROGRAMMERKEYPAD_H
#define UT_PROGRAMMERKEYPAD_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>

#include "../../src/dsettings.h"

class Ut_ProgrammerKeypad : public ::testing::Test
{
public:
    Ut_ProgrammerKeypad();
};

#endif // UT_PROGRAMMERKEYPAD_H
