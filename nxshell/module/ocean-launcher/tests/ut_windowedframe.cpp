// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "windowedframe.h"
#undef private

#include <QTest>

#include <gtest/gtest.h>

class Tst_Windowedframe : public testing::Test
{};

TEST_F(Tst_Windowedframe, privateMethod_test)
{
    WindowedFrame frame;

    frame.moveCurrentSelectApp(Qt::Key_Undo);
    // 循环切换
    for (int count = 0; count < 6; count++) {
        frame.moveCurrentSelectApp(Qt::Key_Tab);
    }
    // 循环回退
    for (int count = 0; count < 6; count++) {
        frame.moveCurrentSelectApp(Qt::Key_Backtab);
    }
    frame.moveCurrentSelectApp(Qt::Key_Up);
    frame.moveCurrentSelectApp(Qt::Key_Down);
    frame.moveCurrentSelectApp(Qt::Key_Left);
    frame.moveCurrentSelectApp(Qt::Key_Right);

    const QModelIndex index;
    frame.switchToCategory(index);

    frame.getCornerPath(WindowedFrame::AnchoredCornor::TopLeft);
    frame.getCornerPath(WindowedFrame::AnchoredCornor::TopRight);
    frame.getCornerPath(WindowedFrame::AnchoredCornor::BottomLeft);
    frame.getCornerPath(WindowedFrame::AnchoredCornor::BottomRight);

    frame.resetWidgetStyle();
    frame.hideLauncher();
}

TEST_F(Tst_Windowedframe, privateSlots_test)
{
    WindowedFrame frame;

    frame.adjustPosition();
    frame.onToggleFullScreen();

    const double value = 0.5;
    frame.onOpacityChanged(value);
    // display model switch
    frame.onSwitchBtnClicked();
    frame.onSwitchBtnClicked();

    frame.onWMCompositeChanged();

    frame.searchText("");
    frame.searchText("test");

    frame.showTips("test");
    frame.prepareHideLauncher();
    frame.recoveryAll();
}

TEST_F(Tst_Windowedframe, eventFilter_test)
{
    WindowedFrame frame;

    QEvent event(QEvent::Type::None);
    frame.m_eventFilter->eventFilter(nullptr, &event);

    QEvent event1(QEvent::Type::KeyPress);
    QKeyEvent *keyPress = static_cast<QKeyEvent *>(&event1);

    QKeyEvent returnKeyEvent(QEvent::Type::KeyPress, Qt::Key_Return, static_cast<QKeyEvent *>(keyPress)->modifiers());
    frame.m_eventFilter->eventFilter(nullptr, &returnKeyEvent);

    QKeyEvent escapeKeyEvent(QEvent::Type::KeyPress, Qt::Key_Escape, static_cast<QKeyEvent *>(keyPress)->modifiers());
    frame.m_eventFilter->eventFilter(nullptr, &escapeKeyEvent);

    QKeyEvent spaceKeyEvent(QEvent::Type::KeyPress, Qt::Key_Space, static_cast<QKeyEvent *>(keyPress)->modifiers());
    frame.m_eventFilter->eventFilter(nullptr, &spaceKeyEvent);

    QKeyEvent tabKeyEvent(QEvent::Type::KeyPress, Qt::Key_Tab, static_cast<QKeyEvent *>(keyPress)->modifiers());
    frame.m_eventFilter->eventFilter(nullptr, &tabKeyEvent);

    QKeyEvent backSpaceKeyEvent(QEvent::Type::KeyPress, Qt::Key_Backspace, static_cast<QKeyEvent *>(keyPress)->modifiers());
    frame.m_eventFilter->eventFilter(nullptr, &backSpaceKeyEvent);
}
