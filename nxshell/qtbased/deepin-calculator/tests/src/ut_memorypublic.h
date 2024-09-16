// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_MEMORYPUBLIC_H
#define UT_MEMORYPUBLIC_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>

#include "../../src/dsettings.h"

class Ut_MemoryPublic : public ::testing::Test
{
public:
    Ut_MemoryPublic();
};

#endif // UT_MEMORYPUBLIC_H
