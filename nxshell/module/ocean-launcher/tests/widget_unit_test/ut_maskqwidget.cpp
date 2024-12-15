// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "maskqwidget.h"
#undef private

#include <QtTest/QtTest>

#include <gtest/gtest.h>

class Tst_Maskqwidget : public testing::Test
{};

TEST_F(Tst_Maskqwidget, hSeparator_test)
{
    MaskQWidget widget;
    QPaintEvent event(QRect(10, 10, 10, 10));
    QApplication::sendEvent(&widget, &event);
}
