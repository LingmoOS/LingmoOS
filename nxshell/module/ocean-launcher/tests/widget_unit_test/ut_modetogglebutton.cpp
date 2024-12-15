// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "modetogglebutton.h"
#undef private

#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTest>

#include <DGuiApplicationHelper>

#include <gtest/gtest.h>

class Tst_Modetogglebutton : public testing::Test
{};

TEST_F(Tst_Modetogglebutton, modeToggleButton_test)
{
    ModeToggleButton button;

    DGuiApplicationHelper::ColorType defaultType = DGuiApplicationHelper::instance()->themeType();

    for (int i = 1; i <= 2; i++) {
        DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::ColorType(i));
        button.resetStyle();
        QTest::qWait(50);
    }

    DGuiApplicationHelper::instance()->setPaletteType(defaultType);
}

TEST_F(Tst_Modetogglebutton, event_test)
{
    ModeToggleButton button;

    button.setFocus();
    button.m_hover = true;
    QPaintEvent paintEvent(QRect(10, 10, 10, 10));
    QApplication::sendEvent(&button, &paintEvent);

    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(&button, &enterEvent);

    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(&button, &leaveEvent);

    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(&button, &releaseEvent);
}
