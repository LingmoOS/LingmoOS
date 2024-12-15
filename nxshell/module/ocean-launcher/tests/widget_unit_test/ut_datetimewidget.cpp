// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "datetimewidget.h"
#undef private

#include <QTest>

#include <gtest/gtest.h>

class Tst_Datetimewidget : public testing::Test
{};

TEST_F(Tst_Datetimewidget, datetimeWidget_test)
{
    DatetimeWidget widget;
    widget.getDateTextWidth();
    widget.updateTime();

    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(&widget, &event);
}
