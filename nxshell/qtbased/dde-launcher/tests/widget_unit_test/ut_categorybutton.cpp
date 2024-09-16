// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appslistmodel.h"
#define private public
#include "categorybutton.h"
#undef private

#include <QApplication>
#include <QWheelEvent>
#include <QTest>

#include <gtest/gtest.h>

class Tst_Categorybutton : public testing::Test
{};

TEST_F(Tst_Categorybutton, bategoryButton_test)
{
    CategoryButton button(AppsListModel::Others);

    QEvent event(QEvent::Enter);
    QApplication::sendEvent(&button, &event);

    button.updateState(CategoryButton::Checked);
    QEvent leaveCheckedEvent(QEvent::Leave);
    QApplication::sendEvent(&button, &leaveCheckedEvent);
    QTest::qWait(10);

    button.updateState(CategoryButton::Normal);
    QEvent leaveNormalEvent(QEvent::Leave);
    QApplication::sendEvent(&button, &leaveNormalEvent);
    QTest::qWait(10);

    QMouseEvent event2(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(&button, &event2);

    button.updateState(CategoryButton::Checked);
    QMouseEvent event3(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(&button, &event3);

    button.updateState(CategoryButton::Normal);
    QMouseEvent normalReleaseEvent(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(&button, &normalReleaseEvent);

    button.updateState(CategoryButton::Checked);
    QApplication::sendEvent(&button, &event3);

    QPaintEvent event4(QRect(10, 10, 10, 10));
    QApplication::sendEvent(&button, &event4);

    button.updateState(CategoryButton::Hover);
    QApplication::sendEvent(&button, &event4);

    button.updateState(CategoryButton::Checked);
    QApplication::sendEvent(&button, &event4);

    button.updateState(CategoryButton::Press);
    QApplication::sendEvent(&button, &event4);
}
