// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "blurboxwidget.h"
#include "appslistmodel.h"
#undef private

#include <QApplication>
#include <QWheelEvent>

#include <gtest/gtest.h>

class Tst_Blurboxwidget : public testing::Test
{};

TEST_F(Tst_Blurboxwidget, blurBoxWidget_test)
{
    BlurBoxWidget widget(AppsListModel::Others, const_cast<char *>("Others"));
    widget.setFixedSize(QSize(10, 10));

    QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(&widget, &event);

    QMouseEvent event1(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(&widget, &event1);

    QWidget w;
    widget.layoutAddWidget(&w, 0, Qt::AlignCenter);

    ASSERT_TRUE(widget.getMultiPagesView());

    QPoint p(0, 0);
    widget.updateBackBlurPos(p);

    widget.setMaskVisible(false);
    widget.setMaskVisible(true);
}
