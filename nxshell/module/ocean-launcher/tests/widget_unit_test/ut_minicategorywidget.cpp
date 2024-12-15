// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "minicategorywidget.h"
#undef private

#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>

#include <gtest/gtest.h>

class Tst_Minicategorywidget : public testing::Test
{};

TEST_F(Tst_Minicategorywidget, miniCategoryWidget_test)
{
    MiniCategoryWidget widget;
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(&widget, &event);

    QEvent event4(QEvent::Enter);
    QApplication::sendEvent(&widget, &event4);

    QEvent event5(QEvent::FocusIn);
    QApplication::sendEvent(&widget, &event5);
    QEvent event6(QEvent::FocusOut);
    QApplication::sendEvent(&widget, &event6);

    widget.onCategoryListChanged();
}
