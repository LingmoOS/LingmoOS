// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "common.h"
#undef private

#include <QPainter>
#include <QWidget>
#include <QTest>

#include <gtest/gtest.h>

class UT_Common : public testing::Test
{

};

class PaintWidget : public QWidget
{

public:
    virtual void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE
    {
        Q_UNUSED(event)
        QPainter painter(this);
        QStyleOptionViewItem option;
        option.rect = QRect(0, 0, 100, 100);
        option.state |= QStyle::State_Selected;
        DrawHelper::DrawImage(&painter, option, "", false);
        DrawHelper::DrawText(&painter, option, "123");
        // FIXME 调用这两个会崩溃
//        DrawHelper::DrawProgressBar(&painter, option, 0.5, Qt::red);
//        DrawHelper::DrawCenterNum(&painter, option, "123", true);
        DrawHelper::DrawBackground(&painter, option);
    }
};

TEST_F(UT_Common, coverageTest)
{
    PaintWidget w;
    w.paintEvent(nullptr);
}
