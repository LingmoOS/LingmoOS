// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_QTEST_H
#define UT_QTEST_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>

#include "../../src/memorypublic.h"
#include "../../src/dsettings.h"

class TestCala : public::testing::Test
{
public:
    TestCala();
};

#endif // UT_QTEST_H
