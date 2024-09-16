// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "gui/grandsearchwidget.h"

#include "interfaces/grandsearchinterface.h"

#include "constants.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <DGuiApplicationHelper>

#include <QPaintEvent>
#include <QMouseEvent>
#include <QPixmap>
#include <QtTest>

using namespace testing;
DGUI_USE_NAMESPACE
GRANDSEARCH_USE_NAMESPACE

TEST(GrandSearchWidgetTest, constructor)
{
    GrandSearchWidget *w = new GrandSearchWidget;

    EXPECT_TRUE(w);
    EXPECT_TRUE(w->m_grandSearchInterface);

    delete w;
}

TEST(GrandSearchWidgetTest, itemCommand)
{
    GrandSearchWidget w;

    w.m_grandSearchVisible = false;

    stub_ext::StubExt stu;

    bool ut_call_setVisible = false;
    stu.set_lamda(ADDR(GrandSearchInterface, SetVisible), [&](){
        ut_call_setVisible = true;
        return QDBusPendingReply<>();
    });

    QString result = w.itemCommand(QString());

    EXPECT_TRUE(result.isEmpty());
    EXPECT_TRUE(ut_call_setVisible);
    EXPECT_TRUE(w.m_grandSearchVisible);
}

TEST(GrandSearchWidgetTest, grandSearchVisibleChanged)
{
    GrandSearchWidget w;

    w.m_grandSearchVisible = false;

    w.grandSearchVisibleChanged(true);

    EXPECT_TRUE(w.m_grandSearchVisible);
}

TEST(GrandSearchWidgetTest, paintEvent)
{
    GrandSearchWidget w;
    QPaintEvent event((QRect()));

    // 1.测试绘制背景阴影
    w.setFixedHeight(PLUGIN_BACKGROUND_MIN_SIZE + 100);

    stub_ext::StubExt stu;

    DGuiApplicationHelper::ColorType ut_themeType = DGuiApplicationHelper::LightType;
    stu.set_lamda(ADDR(DGuiApplicationHelper, themeType), [&](){
        return ut_themeType;
    });

    bool ut_call_loadSvg = false;
    stu.set_lamda(&GrandSearchWidget::loadSvg, [&]() {
        ut_call_loadSvg = true;
        return QPixmap();
    });
    // 1.1 浅色主题
    w.m_hover = true;
    w.m_pressed = true;

    w.paintEvent(&event);
    EXPECT_TRUE(ut_call_loadSvg);

    // 1.2 深色主题
    ut_themeType = DGuiApplicationHelper::DarkType;
    ut_call_loadSvg = false;

    w.paintEvent(&event);
    EXPECT_TRUE(ut_call_loadSvg);

    // 2.测试最小尺寸无背景阴影
    w.setFixedHeight(PLUGIN_BACKGROUND_MIN_SIZE - 100);
    ut_themeType = DGuiApplicationHelper::LightType;
    ut_call_loadSvg = false;

    w.paintEvent(&event);
    EXPECT_TRUE(ut_call_loadSvg);
}

TEST(GrandSearchWidgetTest, mousePressEvent)
{
    GrandSearchWidget w;

    w.m_pressed = false;

    QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);

    w.mousePressEvent(&event);

    EXPECT_TRUE(w.m_pressed);
}

TEST(GrandSearchWidgetTest, mouseReleaseEvent)
{
    GrandSearchWidget w;

    w.m_pressed = true;
    w.m_hover = true;

    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(0, 0), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);

    w.mouseReleaseEvent(&event);

    EXPECT_FALSE(w.m_pressed);
    EXPECT_FALSE(w.m_hover);
}

TEST(GrandSearchWidgetTest, mouseMoveEvent)
{
    GrandSearchWidget w;

    w.m_hover = false;

    QMouseEvent event(QEvent::MouseMove, QPointF(0, 0), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    w.mouseMoveEvent(&event);

    EXPECT_TRUE(w.m_hover);
}

TEST(GrandSearchWidgetTest, leaveEvent)
{
    GrandSearchWidget w;

    w.m_hover = true;
    w.m_pressed = true;

    QEvent event(QEvent::Leave);

    w.leaveEvent(&event);

    EXPECT_FALSE(w.m_hover);
    EXPECT_FALSE(w.m_pressed);
}

TEST(GrandSearchWidgetTest, loadSvg)
{
    GrandSearchWidget w;

    QString fileName("testFileName");
    QSize size(100, 100);

    EXPECT_NO_FATAL_FAILURE(w.loadSvg(fileName, size));
}
