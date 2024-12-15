// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QtTest>
#include <QColor>
#include <QDebug>
#include <gtest/gtest.h>
#include "../widgets/colorbutton.h"


using namespace testing;

class ColorButtonTest:public testing::Test, public QObject{

public:
    ColorButton *button;
    virtual void SetUp() override{
        std::cout << "start ColorButtonTest" << std::endl;
        button  = new ColorButton(QColor("#ff1c49"), nullptr);
    }

    virtual void TearDown() override{
        delete button;
        std::cout << "end ColorButtonTest" << std::endl;
    }
};
TEST_F(ColorButtonTest, ColorButton)
{
    connect(button, &ColorButton::updatePaintColor, this, [ = ](QColor color) {
        qDebug() << color;
        EXPECT_EQ(color, QColor("#ff1c49"));
    });
    QTest::mouseClick(button,Qt::MouseButton::LeftButton);
}
