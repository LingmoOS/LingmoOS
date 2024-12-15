// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/dsplitlistwidget.h"

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"
#include "utils.h"
#include "commonheaderfile.h"

#include "globaldef.h"
#include <QSignalSpy>
#include <QPainter>
#include <QMouseEvent>
#include <QToolTip>
#include <QTest>

#include <DStyleHelper>
#include <DApplication>
#include <DApplicationHelper>
#include <DLog>
#include <DFontSizeManager>


namespace {
class TestDSplitListWidget : public testing::Test
{

protected:
    void SetUp()
    {
        dsp = new DSplitListWidget(w);
    }
    void TearDown()
    {
        delete dsp;
    }

    // Some expensive resource shared by all tests.
    QWidget *w = new QWidget;
    DSplitListWidget *dsp;
};

int stub_returnRow()
{
    return 5;
}

static QString g_funcname;
void stub_paintBackground(QPainter *, const QStyleOptionViewItem &, const QRect &, const QPalette::ColorGroup)
{
    g_funcname = __FUNCTION__;
}
void stub_fillRect(void *, const QRect &, const QColor &)
{
    g_funcname = __FUNCTION__;
}
void stub_hideTooltipImmediately()
{
    g_funcname = __FUNCTION__;
}
}

TEST_F(TestDSplitListWidget, checkMousePressEvent)
{
    QTest::mouseClick(dsp->viewport(), Qt::LeftButton);
    EXPECT_TRUE(dsp->m_IsMouseClicked);

    QModelIndex modelIndex = dsp->m_categoryItemModell->index(1, 0);
    dsp->setCurrentIndex(modelIndex);

    QTest::mouseClick(dsp->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(50, 50));
    EXPECT_TRUE(dsp->m_IsMouseClicked);
    EXPECT_TRUE(dsp->lastTouchBeginPos == QPoint(50, 50));
}

TEST_F(TestDSplitListWidget, checkHelpEvent)
{
    QHelpEvent *e = new QHelpEvent(QEvent::ToolTip, QPoint(50, 50), QPoint(600, 400));

    QModelIndex modelIndex = dsp->m_categoryItemModell->index(1, 0);

    QStyleOptionViewItem option;
    EXPECT_FALSE(dsp->itemDelegate()->helpEvent(e, dsp, option, modelIndex));

    Stub s;
    s.set(ADDR(DNoFocusDelegate, hideTooltipImmediately), stub_hideTooltipImmediately);

    QHelpEvent *e1 = new QHelpEvent(QEvent::ToolTip, QPoint(45, 200), QPoint(600, 500));
    modelIndex = dsp->m_categoryItemModell->index(5, 0);
    EXPECT_FALSE(dsp->itemDelegate()->helpEvent(e1, dsp, option, modelIndex));
    EXPECT_TRUE(g_funcname == QLatin1String("stub_hideTooltipImmediately"));


    SAFE_DELETE_ELE(e)
    SAFE_DELETE_ELE(e1)
}

TEST_F(TestDSplitListWidget, checkEventFilter)
{
    QEvent *e = new QEvent(QEvent::FocusOut);

    dsp->eventFilter(dsp, e);
    EXPECT_FALSE(dsp->m_IsMouseClicked);
    EXPECT_FALSE(dsp->m_IsTabFocus);
    EXPECT_FALSE(dsp->m_IsLeftFocus);
    EXPECT_FALSE(dsp->m_IsHalfWayFocus);

    QEvent *e1 = new QEvent(QEvent::FocusIn);
    dsp->eventFilter(dsp, e1);
    EXPECT_FALSE(dsp->m_IsTabFocus);

    dsp->eventFilter(dsp, e1);
    EXPECT_TRUE(dsp->m_IsTabFocus);

    SAFE_DELETE_ELE(e)
    SAFE_DELETE_ELE(e1)
}

TEST_F(TestDSplitListWidget, checkKyPressEvent)
{
    QModelIndex modelIndex = dsp->m_categoryItemModell->index(0, 0);
    dsp->setCurrentIndex(modelIndex);
    QTest::keyPress(dsp->viewport(), Qt::Key_Up);
    EXPECT_TRUE(dsp->currentIndex().row() == 7);

    modelIndex = dsp->m_categoryItemModell->index(1, 0);
    dsp->setCurrentIndex(modelIndex);
    QTest::keyPress(dsp->viewport(), Qt::Key_Up);
    EXPECT_TRUE(dsp->currentIndex().row() == 0);

    modelIndex = dsp->m_categoryItemModell->index(7, 0);
    dsp->setCurrentIndex(modelIndex);
    QTest::keyPress(dsp->viewport(), Qt::Key_Down);
    EXPECT_TRUE(dsp->currentIndex().row() == 0);

    modelIndex = dsp->m_categoryItemModell->index(6, 0);
    dsp->setCurrentIndex(modelIndex);
    QTest::keyPress(dsp->viewport(), Qt::Key_Down);
    EXPECT_TRUE(dsp->currentIndex().row() == 7);

    QTest::keyPress(dsp->viewport(), Qt::Key_Alt);

}

TEST_F(TestDSplitListWidget, checkWheelEventDeltaP)
{
    QWheelEvent *e = new QWheelEvent(QPoint(45, 200), QPoint(600, 500), 10, Qt::NoButton, Qt::NoModifier);
    QModelIndex modelIndex = dsp->m_categoryItemModell->index(1, 0);
    dsp->setCurrentIndex(modelIndex);
    dsp->wheelEvent(e);
    EXPECT_TRUE(dsp->currentIndex().row() == 0);


    modelIndex = dsp->m_categoryItemModell->index(6, 0);
    dsp->setCurrentIndex(modelIndex);
    dsp->wheelEvent(e);
    EXPECT_TRUE(dsp->currentIndex().row() == 4);

    modelIndex = dsp->m_categoryItemModell->index(0, 0);
    dsp->setCurrentIndex(modelIndex);
    dsp->wheelEvent(e);
    EXPECT_TRUE(dsp->currentIndex().row() == 0);

    dsp->setRefreshFinished(true);
    modelIndex = dsp->m_categoryItemModell->index(2, 0);
    dsp->setCurrentIndex(modelIndex);
    dsp->wheelEvent(e);
    EXPECT_TRUE(dsp->currentIndex().row() == 2);

    SAFE_DELETE_ELE(e)
}

TEST_F(TestDSplitListWidget, checkWheelEventDeltaN)
{
    QWheelEvent *e = new QWheelEvent(QPoint(45, 200), QPoint(600, 500), -10, Qt::NoButton, Qt::NoModifier);
    QModelIndex modelIndex = dsp->m_categoryItemModell->index(4, 0);
    dsp->setCurrentIndex(modelIndex);
    dsp->wheelEvent(e);
    EXPECT_TRUE(dsp->currentIndex().row() == 6);


    modelIndex = dsp->m_categoryItemModell->index(3, 0);
    dsp->setCurrentIndex(modelIndex);
    dsp->wheelEvent(e);
    EXPECT_TRUE(dsp->currentIndex().row() == 4);

    modelIndex = dsp->m_categoryItemModell->index(dsp->count() - 1, 0);
    dsp->setCurrentIndex(modelIndex);
    dsp->wheelEvent(e);
    EXPECT_TRUE(dsp->currentIndex().row() == dsp->count() - 1);

    SAFE_DELETE_ELE(e)
}


TEST_F(TestDSplitListWidget, checkMouseMoveEvent)
{
    QMouseEvent *e = new QMouseEvent(QEvent::MouseMove, QPoint(600, 500), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);

    dsp->mouseMoveEvent(e);
    EXPECT_TRUE(dsp->m_isMouseMoved);

    QHelpEvent *e2 = new QHelpEvent(QEvent::ToolTip, QPoint(50, 50), QPoint(600, 400));
    QModelIndex modelIndex = dsp->m_categoryItemModell->index(1, 0);
    QStyleOptionViewItem option;

    EXPECT_FALSE(dsp->itemDelegate()->helpEvent(e2, dsp, option, modelIndex));
    dsp->mouseMoveEvent(e);
    EXPECT_TRUE(dsp->m_isMouseMoved);
    EXPECT_TRUE(dsp->m_IsPositive);

    SAFE_DELETE_ELE(e)
    SAFE_DELETE_ELE(e2)

}

TEST_F(TestDSplitListWidget, checkGetSetStatus)
{
    FocusStatus s1;
    s1.m_IsMouseClicked = false;
    s1.m_IsFirstFocus = false;
    s1.m_IsHalfWayFocus = true;

    dsp->setCurrentStatus(s1);

    EXPECT_FALSE(dsp->m_currentStatus.m_IsFirstFocus);
    EXPECT_FALSE(dsp->m_currentStatus.m_IsMouseClicked);
    EXPECT_TRUE(dsp->m_currentStatus.m_IsHalfWayFocus);

    FocusStatus s2;

    s2 = dsp->getStatus();
    EXPECT_FALSE(s2.m_IsHalfWayFocus);
    EXPECT_FALSE(s2.m_IsMouseClicked);
    EXPECT_TRUE(s2.m_IsFirstFocus);

}

TEST_F(TestDSplitListWidget, checkPaint)
{
    QPainter *p = new QPainter;
    QStyleOptionViewItem option;
    QModelIndex modelIndex1 = dsp->m_categoryItemModell->index(4, 0);

    Stub s;
    s.set(ADDR(DNoFocusDelegate, paintBackground), stub_paintBackground);
    dsp->itemDelegate()->paint(p, option, modelIndex1);
    EXPECT_TRUE(g_funcname == QLatin1String("stub_paintBackground"));

    option.state = QStyle::State_Selected;
    dsp->m_IsTabFocus = true;
    dsp->itemDelegate()->paint(p, option, modelIndex1);
    EXPECT_TRUE(g_funcname == QLatin1String("stub_paintBackground"));

    option.state.setFlag(QStyle::State_MouseOver);
    dsp->m_IsTabFocus = true;


    dsp->m_IsTabFocus = false;
    option.state.setFlag(QStyle::State_Selected);
    dsp->itemDelegate()->paint(p, option, modelIndex1);
    EXPECT_TRUE(g_funcname == QLatin1String("stub_paintBackground"));

    option.state.setFlag(QStyle::State_MouseOver);
    dsp->itemDelegate()->paint(p, option, modelIndex1);
    EXPECT_TRUE(g_funcname == QLatin1String("stub_paintBackground"));

    dsp->m_IsTabFocus = true;
    option.state.setFlag(QStyle::State_MouseOver);
    dsp->itemDelegate()->paint(p, option, modelIndex1);
    EXPECT_TRUE(g_funcname == QLatin1String("stub_paintBackground"));

    option.state = QStyle::State_MouseOver;
    dsp->m_IsTabFocus = true;
    dsp->itemDelegate()->paint(p, option, modelIndex1);
    EXPECT_TRUE(g_funcname == QLatin1String("stub_paintBackground"));

    s.set((void(QPainter::*)(const QRect &, const QColor &))ADDR(QPainter, fillRect), stub_fillRect);
    modelIndex1 = dsp->m_categoryItemModell->index(5, 0);
    dsp->itemDelegate()->paint(p, option, modelIndex1);
    EXPECT_TRUE(g_funcname == QLatin1String("stub_fillRect"));

    SAFE_DELETE_ELE(p)
}

TEST_F(TestDSplitListWidget, checkMouseReleaseEvent)
{
    dsp->m_isIstalling = true;
    QMouseEvent *e = new QMouseEvent(QEvent::MouseButtonRelease, QPoint(), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    dsp->mouseMoveEvent(e);

    Stub s;
    s.set(ADDR(QModelIndex, row), stub_returnRow);

    dsp->m_isIstalling = false;
    dsp->mouseMoveEvent(e);
    EXPECT_FALSE(dsp->m_IsPositive);
    SAFE_DELETE_ELE(e)
}







