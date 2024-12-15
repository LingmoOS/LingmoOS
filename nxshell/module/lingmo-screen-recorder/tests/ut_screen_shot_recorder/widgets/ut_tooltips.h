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
#include "../../src/widgets/tooltips.h"

using namespace testing;
ACCESS_PRIVATE_FUN(ToolTips, void(QResizeEvent *ev), resizeEvent);
ACCESS_PRIVATE_FUN(ToolTips, void(QEvent *e), enterEvent);
ACCESS_PRIVATE_FUN(ToolTips, void(QPaintEvent *), paintEvent);
DGuiApplicationHelper::ColorType themeType_default_stub()
{
    return DGuiApplicationHelper::ColorType::UnknownType;
}
DGuiApplicationHelper::ColorType themeType_Light_stub()
{
    return DGuiApplicationHelper::ColorType::LightType;
}
DGuiApplicationHelper::ColorType themeType_Dark_stub()
{
    return DGuiApplicationHelper::ColorType::DarkType;
}

class ToolTipsTest:public testing::Test{

public:
    Stub stub;
    ToolTips *m_toolTips;
    QString m_tips = QString("");
    virtual void SetUp() override{
        m_toolTips = new ToolTips(m_tips);
    }

    virtual void TearDown() override{
        if(nullptr != m_toolTips)
            delete m_toolTips;
    }
};

TEST_F(ToolTipsTest, radius)
{
    m_toolTips->radius();
}

TEST_F(ToolTipsTest, enterEvent)
{
    QEvent *e = new QEvent(QEvent::ToolTip);
    call_private_fun::ToolTipsenterEvent(*m_toolTips,e);
    delete e;
}

TEST_F(ToolTipsTest, borderColor)
{
    m_toolTips->borderColor();
}

TEST_F(ToolTipsTest, background)
{
    m_toolTips->background();
}

TEST_F(ToolTipsTest, setText)
{
    m_toolTips->setText(QString("tips"));
}

TEST_F(ToolTipsTest, setRadius)
{
    m_toolTips->setRadius(2);
}

TEST_F(ToolTipsTest, setVertical)
{
    m_toolTips->setVertical();
}

TEST_F(ToolTipsTest, resetSize)
{
    m_toolTips->resetSize(2);
}

TEST_F(ToolTipsTest, setBackground)
{
    m_toolTips->setBackground(QBrush());
}

TEST_F(ToolTipsTest, setBorderColor)
{
    m_toolTips->setBorderColor(QColor());
}

TEST_F(ToolTipsTest, resizeEvent)
{
    QResizeEvent *resizeEvent = new QResizeEvent(QSize(100,100),QSize(200,200));
    call_private_fun::ToolTipsresizeEvent(*m_toolTips,resizeEvent);
    delete resizeEvent;
}

TEST_F(ToolTipsTest, paintEvent)
{
    QPaintEvent *paintEvent = new QPaintEvent(QRect());
    stub.set(ADDR(DGuiApplicationHelper,themeType),themeType_default_stub);
    call_private_fun::ToolTipspaintEvent(*m_toolTips,paintEvent);

    stub.reset(ADDR(DGuiApplicationHelper,themeType));
    stub.set(ADDR(DGuiApplicationHelper,themeType),themeType_Light_stub);
    call_private_fun::ToolTipspaintEvent(*m_toolTips,paintEvent);

    stub.reset(ADDR(DGuiApplicationHelper,themeType));
    stub.set(ADDR(DGuiApplicationHelper,themeType),themeType_Dark_stub);
    call_private_fun::ToolTipspaintEvent(*m_toolTips,paintEvent);

    stub.reset(ADDR(DGuiApplicationHelper,themeType));

    delete paintEvent;
}







