// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "categorytitlewidget.h"
#undef private

#include <QTest>

#include <gtest/gtest.h>

class Tst_Categorytitlewidget : public testing::Test
{};

TEST_F(Tst_Categorytitlewidget, categoryTitleWidget_test)
{
    CategoryTitleWidget widget("test");

    widget.updatePosition(QPoint(0, 0), 10, 1);
    widget.updatePosition(QPoint(0, 0), 10, 2);
    widget.updatePosition(QPoint(0, 0), 10, 4);
    widget.updatePosition(QPoint(0, 0), 10, 5);

    QVERIFY(widget.textLabel()->text() == "test");
}
