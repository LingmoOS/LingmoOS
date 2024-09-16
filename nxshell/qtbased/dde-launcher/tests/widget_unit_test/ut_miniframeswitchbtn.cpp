// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "windowedframe.h"

#define private public
#include "miniframeswitchbtn.h"
#undef private

#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <DGuiApplicationHelper>

#include <gtest/gtest.h>

class Tst_Miniframeswitchbtn : public testing::Test
{};

TEST_F(Tst_Miniframeswitchbtn, miniFrameSwitchBtn_test)
{
    MiniFrameSwitchBtn button;

    button.updateStatus(WindowedFrame::All);
    button.updateStatus(WindowedFrame::Category);
    button.click();

    for (int i = 0; i <= 2; i++) {
        DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::ColorType(i));
        button.updateIcon();
    }
}

TEST_F(Tst_Miniframeswitchbtn, event_test)
{
    MiniFrameSwitchBtn button;

    DGuiApplicationHelper::ColorType defaultType = DGuiApplicationHelper::instance()->themeType();

    QPaintEvent paintEvent(QRect(10, 10, 10, 10));
    QApplication::sendEvent(&button, &paintEvent);

    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(&button, &enterEvent);

    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(&button, &leaveEvent);

    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(&button, &releaseEvent);

    button.setFocus();
    QApplication::sendEvent(&button, &paintEvent);

    DGuiApplicationHelper::instance()->setPaletteType(defaultType);
}
