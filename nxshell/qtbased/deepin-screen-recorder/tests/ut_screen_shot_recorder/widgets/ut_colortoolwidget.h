// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QtTest>
#include <QColor>
#include <QDebug>
#include <gtest/gtest.h>
#include "../../src/widgets/colortoolwidget.h"
#include "addr_pri.h"


using namespace testing;

ACCESS_PRIVATE_FIELD(ColorToolWidget, ToolButton*, m_redBtn);
ACCESS_PRIVATE_FIELD(ColorToolWidget, ToolButton*, m_yellowBtn);
ACCESS_PRIVATE_FIELD(ColorToolWidget, ToolButton*, m_blueBtn);
ACCESS_PRIVATE_FIELD(ColorToolWidget, ToolButton*, m_greenBtn);

class ColorToolWidgetTest:public testing::Test, public QObject{

public:
    ColorToolWidget *widget;
    QString curColor;
    virtual void SetUp() override{
        std::cout << "start ColorToolWidgetTest" << std::endl;
        widget  = new ColorToolWidget(nullptr);
        connect(widget, &ColorToolWidget::colorChecked, this, &ColorToolWidgetTest::OnColorChecked);
    }

    virtual void TearDown() override{
        delete widget;
        std::cout << "end ColorToolWidgetTest" << std::endl;
    }
public slots:
    void OnColorChecked(QString color);
};
TEST_F(ColorToolWidgetTest, colorChecked)
{

    ToolButton* red = access_private_field::ColorToolWidgetm_redBtn(*widget);
    ToolButton* yellow = access_private_field::ColorToolWidgetm_yellowBtn(*widget);
    ToolButton* blue = access_private_field::ColorToolWidgetm_blueBtn(*widget);
    ToolButton* green = access_private_field::ColorToolWidgetm_greenBtn(*widget);
    curColor = "red";
    QTest::mouseClick(red, Qt::MouseButton::LeftButton);
    curColor = "yellow";
    QTest::mouseClick(yellow, Qt::MouseButton::LeftButton);
    curColor = "blue";
    QTest::mouseClick(blue, Qt::MouseButton::LeftButton);
    curColor = "green";
    QTest::mouseClick(green, Qt::MouseButton::LeftButton);
}
void ColorToolWidgetTest::OnColorChecked(QString color)
{
    EXPECT_EQ(color, curColor);
    //qDebug() << "OnColorChecked" << color;
}
