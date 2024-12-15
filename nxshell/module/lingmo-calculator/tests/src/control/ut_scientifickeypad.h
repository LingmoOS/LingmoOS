// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_SCIENTIFICKEYPAD_H
#define UT_SCIENTIFICKEYPAD_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>

#include "../../src/dsettings.h"
#include"../stub.h"

class Ut_ScientificKeypad : public ::testing::Test
{
public:
    Ut_ScientificKeypad();
};

#endif // UT_SCIENTIFICKEYPAD_H
