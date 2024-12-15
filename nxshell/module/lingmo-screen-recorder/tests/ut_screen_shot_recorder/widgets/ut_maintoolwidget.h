// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QtTest>
#include <QColor>
#include <QDebug>
#include <gtest/gtest.h>
#include "../../src/widgets/maintoolwidget.h"
#include "addr_pri.h"


using namespace testing;

ACCESS_PRIVATE_FIELD(MainToolWidget, ToolButton*, m_recordBtn);
ACCESS_PRIVATE_FIELD(MainToolWidget, ToolButton*, m_shotBtn);

class MainToolWidgetTest:public testing::Test, public QObject{

public:
    MainToolWidget *widget;
    QString curType;
    virtual void SetUp() override{
        std::cout << "start MainToolWidgetTest" << std::endl;
        widget  = new MainToolWidget(nullptr);
        connect(widget, &MainToolWidget::buttonChecked, this, &MainToolWidgetTest::buttonChecked);
    }

    virtual void TearDown() override{
        delete widget;
        std::cout << "end MainToolWidgetTest" << std::endl;
    }
public slots:
    void buttonChecked(bool checked, QString type);
};
TEST_F(MainToolWidgetTest, colorChecked)
{
    ToolButton* recorBtn = access_private_field::MainToolWidgetm_recordBtn(*widget);
    ToolButton* recorShot = access_private_field::MainToolWidgetm_shotBtn(*widget);
    curType = "record";
    QTest::mouseClick(recorBtn, Qt::MouseButton::LeftButton);
    curType = "shot";
    QTest::mouseClick(recorShot, Qt::MouseButton::LeftButton);

}
void MainToolWidgetTest::buttonChecked(bool checked, QString type)
{
    EXPECT_EQ(type, curType);
}
