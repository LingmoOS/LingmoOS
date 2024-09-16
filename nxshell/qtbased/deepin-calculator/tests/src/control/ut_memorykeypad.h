// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_MEMORYKEYPAD_H
#define UT_MEMORYKEYPAD_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>

#include "../../src/dsettings.h"

class Ut_MemoryKeypad : public ::testing::Test
{
public:
    Ut_MemoryKeypad();
};

#endif // UT_MEMORYKEYPAD_H
