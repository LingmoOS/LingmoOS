// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_MEMORYITEMDELEGATE_H
#define UT_MEMORYITEMDELEGATE_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>
#include <DApplicationHelper>
#include"../stub.h"
#include "../../src/views/memoryitemdelegate.h"

class Ut_MemoryItemDelegate : public ::testing::Test
{
public:
    Ut_MemoryItemDelegate();
    MemoryItemDelegate *m_memoryItemDelegate;

protected:
    void SetUp();
    void TearDown();
};

#endif // UT_MEMORYITEMDELEGATE_H
