// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "applistarea.h"

#include <QApplication>
#include <QWheelEvent>

#include <gtest/gtest.h>

class Tst_Applistarea : public testing::Test
{};

TEST_F(Tst_Applistarea, appListArea_test)
{
    AppListArea area;
    QWheelEvent event(QPointF(0, 0), 0, Qt::MiddleButton, Qt::ControlModifier);
    QApplication::sendEvent(&area, &event);

    QEvent event1(QEvent::Enter);
    QApplication::sendEvent(&area, &event1);

    QMouseEvent event2(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(&area, &event2);

    QMouseEvent event3(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(&area, &event3);

    QWheelEvent wheelEvent(QPointF(0, 0), 10, Qt::MiddleButton, Qt::ControlModifier);
    QApplication::sendEvent(&area, &wheelEvent);

    QWheelEvent wheelEvent2(QPointF(0, 0), -10, Qt::MiddleButton, Qt::ControlModifier);
    QApplication::sendEvent(&area, &wheelEvent2);

}
