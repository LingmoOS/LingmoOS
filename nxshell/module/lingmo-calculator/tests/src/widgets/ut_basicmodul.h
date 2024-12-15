// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_BASICMODUL_H
#define UT_BASICMODUL_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>

#include "../../src/memorypublic.h"
#include "../../src/dsettings.h"

class Ut_BasicModul : public ::testing::Test
{
public:
    Ut_BasicModul();

protected:
};

#endif // UT_BASICMODUL_H
