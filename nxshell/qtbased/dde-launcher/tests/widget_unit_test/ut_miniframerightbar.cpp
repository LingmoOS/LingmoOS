// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "miniframerightbar.h"
#undef private

#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTest>
#include <DGuiApplicationHelper>

#include <gtest/gtest.h>

DGUI_USE_NAMESPACE

class Tst_Miniframerightbar : public testing::Test
{};

TEST_F(Tst_Miniframerightbar, miniFrameRightBar_test)
{
    MiniFrameRightBar bar;
    DGuiApplicationHelper::ColorType defaultType = DGuiApplicationHelper::instance()->themeType();

    bar.setCurrentCheck(true);
    bar.moveUp();
    bar.moveDown();
    bar.execCurrent();

    for (int i = 0; i <= 2; i++) {
        DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::ColorType(i));
        QPaintEvent event(QRect(10, 10, 10, 10));
        QApplication::sendEvent(&bar, &event);
        QTest::qWait(10);
    }

    DGuiApplicationHelper::instance()->setPaletteType(defaultType);
}
