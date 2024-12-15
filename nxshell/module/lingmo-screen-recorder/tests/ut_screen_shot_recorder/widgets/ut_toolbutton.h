// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>
#include "stub.h"
#include "addr_pri.h"
#include <DApplicationHelper>
#include "../../src/widgets/toolbutton.h"

using namespace testing;
//ACCESS_PRIVATE_FUN(ToolTips, void(QResizeEvent *ev), resizeEvent);
ACCESS_PRIVATE_FUN(ToolButton, void(QEvent *e), enterEvent);
ACCESS_PRIVATE_FUN(ToolButton, void(QEvent *e), leaveEvent);
ACCESS_PRIVATE_FUN(ToolButton, void(QMouseEvent *e), mousePressEvent);
//ACCESS_PRIVATE_FUN(ToolTips, void(QPaintEvent *), paintEvent);
//DGuiApplicationHelper::ColorType themeType_default_stub()
//{
//    return DGuiApplicationHelper::ColorType::UnknownType;
//}
//DGuiApplicationHelper::ColorType themeType_Light_stub()
//{
//    return DGuiApplicationHelper::ColorType::LightType;
//}
//DGuiApplicationHelper::ColorType themeType_Dark_stub()
//{
//    return DGuiApplicationHelper::ColorType::DarkType;
//}

class ToolButtonTest:public testing::Test{

public:
    Stub stub;
    ToolButton *m_toolButton;
    virtual void SetUp() override{
        m_toolButton = new ToolButton();
    }

    virtual void TearDown() override{
        if(nullptr != m_toolButton)
            delete m_toolButton;
    }
};

TEST_F(ToolButtonTest, setTips)
{
    m_toolButton->setTips(QString("tips"));
}

TEST_F(ToolButtonTest, getTips)
{
    EXPECT_NE(QString("tips"),m_toolButton->getTips());
}

TEST_F(ToolButtonTest, enterEvent)
{
    QEvent *e = new QEvent(QEvent::ToolTip);
    call_private_fun::ToolButtonenterEvent(*m_toolButton,e);
    delete e;
}

TEST_F(ToolButtonTest, leaveEvent)
{
    QEvent *e = new QEvent(QEvent::ToolTip);
    call_private_fun::ToolButtonleaveEvent(*m_toolButton,e);
    delete e;
}

TEST_F(ToolButtonTest, mousePressEvent)
{
    QMouseEvent *e = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10,10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    call_private_fun::ToolButtonmousePressEvent(*m_toolButton,e);
    delete e;
}






