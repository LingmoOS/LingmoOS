// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "miniframebutton.h"
#undef private

#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <DGuiApplicationHelper>
#include <QRect>

#include <gtest/gtest.h>

DGUI_USE_NAMESPACE

class Tst_Miniframebutton : public testing::Test
{};

TEST_F(Tst_Miniframebutton, miniFrameButton_test)
{
    MiniFrameButton button("OK");

    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(&button, &enterEvent);

    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(&button, &leaveEvent);

    QEvent fontEvent(QEvent::ApplicationFontChange);
    QApplication::sendEvent(&button, &fontEvent);

    button.setChecked(true);
    button.setIcon(QIcon(":/icons/skin/icons/clear_36px.svg"));
    QPaintEvent paintEvent(QRect(10, 10, 10, 10));
    QApplication::sendEvent(&button, &paintEvent);
}

TEST_F(Tst_Miniframebutton, themeTypeChange_test)
{
    MiniFrameButton button("OK");

    int themeType = DGuiApplicationHelper::instance()->themeType();

    DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::DarkType);
    button.onThemeTypeChanged(DGuiApplicationHelper::instance()->themeType());

    // 恢复模式值
    DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::ColorType(themeType));
}

