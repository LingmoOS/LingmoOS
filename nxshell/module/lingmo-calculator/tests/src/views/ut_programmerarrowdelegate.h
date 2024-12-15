// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_PROGRAMMERARROWDELEGATE_H
#define UT_PROGRAMMERARROWDELEGATE_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>
#include <DApplicationHelper>
#include"../stub.h"
#include "../../src/views/programmerarrowdelegate.h"

class Ut_ProgrammerArrowDelegate : public ::testing::Test
{
public:
    Ut_ProgrammerArrowDelegate();

    ProgrammerArrowDelegate *m_memoryItemDelegate;

protected:
    void SetUp();
    void TearDown();
};

#endif // UT_PROGRAMMERARROWDELEGATE_H
