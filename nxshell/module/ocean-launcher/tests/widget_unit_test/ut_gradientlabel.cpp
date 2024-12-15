// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "gradientlabel.h"
#undef private

#include <QTest>

#include <gtest/gtest.h>

class Tst_Gradientlabel : public testing::Test
{};

TEST_F(Tst_Gradientlabel, gradientLabel_test)
{
    GradientLabel label;
    QVERIFY(GradientLabel::TopToBottom == label.direction());
    QPixmap pix(10, 10);

    label.setDirection(GradientLabel::TopToBottom);
    QVERIFY(GradientLabel::TopToBottom == label.direction());
    label.setPixmap(pix);
    label.setDirection(GradientLabel::BottomToTop);
    QVERIFY(GradientLabel::BottomToTop == label.direction());
    label.setPixmap(pix);
    label.setDirection(GradientLabel::LeftToRight);
    QVERIFY(GradientLabel::LeftToRight == label.direction());
    label.setPixmap(pix);
    label.setDirection(GradientLabel::RightToLeft);
    QVERIFY(GradientLabel::RightToLeft == label.direction());
    label.setPixmap(pix);

    QPaintEvent event4(QRect(10, 10, 10, 10));
    QApplication::sendEvent(&label, &event4);
}
