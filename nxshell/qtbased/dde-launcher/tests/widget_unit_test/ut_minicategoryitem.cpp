// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "minicategorywidget.h"

#include <QTest>

#include "gtest/gtest.h"

class Tst_MiniCategoryItem : public testing::Test
{};

TEST_F(Tst_MiniCategoryItem, itemIsCheckable_test)
{
    MiniCategoryItem item((QString()));
    QCOMPARE(item.isCheckable(), true);
}
