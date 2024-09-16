// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "folderselectdialog.h"
#include "ut_folderselectview.h"
#include "leftview.h"
#include "leftviewdelegate.h"
#include "leftviewsortfilter.h"

#include <QStandardItemModel>

UT_FolderSelectView::UT_FolderSelectView()
{
}

TEST_F(UT_FolderSelectView, UT_FolderSelectView_mousePressEvent_001)
{
    FolderSelectView selectview;
    QPointF localPos(30, 20);
    QMouseEvent *mousePressEvent = new QMouseEvent(QEvent::MouseButtonPress, localPos, localPos, localPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSource::MouseEventSynthesizedByQt);
    selectview.mousePressEvent(mousePressEvent);
    EXPECT_EQ(mousePressEvent->pos().y(), selectview.m_touchPressPointY)
        << "event->pos().y()";
    delete mousePressEvent;
}

TEST_F(UT_FolderSelectView, UT_FolderSelectView_mousePressEvent_002)
{
    FolderSelectView selectview;
    QPointF localPos(30, 20);
    QMouseEvent *e = new QMouseEvent(QEvent::MouseButtonPress, localPos, localPos, localPos,
                                     Qt::LeftButton, Qt::LeftButton, Qt::NoModifier,
                                     Qt::MouseEventSource::MouseEventNotSynthesized);
    selectview.mousePressEvent(e);
    delete e;
}

TEST_F(UT_FolderSelectView, UT_FolderSelectView_mouseReleaseEvent_001)
{
    FolderSelectView selectview;
    QPointF localPos(30, 20);
    QMouseEvent *mousePressEvent = new QMouseEvent(QEvent::MouseButtonRelease, localPos, localPos, localPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSource::MouseEventSynthesizedByQt);
    selectview.mouseReleaseEvent(mousePressEvent);
    EXPECT_FALSE(selectview.m_isTouchSliding);
    delete mousePressEvent;
}

TEST_F(UT_FolderSelectView, UT_FolderSelectView_mouseReleaseEvent_002)
{
    FolderSelectView selectview;
    QPointF localPos(30, 20);
    QMouseEvent *mousePressEvent = new QMouseEvent(QEvent::MouseButtonRelease, localPos, localPos, localPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSource::MouseEventSynthesizedByQt);
    selectview.m_isTouchSliding = true;
    selectview.mouseReleaseEvent(mousePressEvent);
    EXPECT_FALSE(selectview.m_isTouchSliding);
    delete mousePressEvent;
}

TEST_F(UT_FolderSelectView, UT_FolderSelectView_mouseReleaseEvent_003)
{
    FolderSelectView selectview;
    QPointF localPos(30, 20);
    QMouseEvent *mousePressEvent = new QMouseEvent(QEvent::MouseButtonRelease, localPos, localPos, localPos,
                                                   Qt::LeftButton, Qt::LeftButton, Qt::NoModifier,
                                                   Qt::MouseEventSource::MouseEventNotSynthesized);
    selectview.mouseReleaseEvent(mousePressEvent);
    delete mousePressEvent;
}

TEST_F(UT_FolderSelectView, UT_FolderSelectView_keyPressEvent_001)
{
    FolderSelectView selectview;
    QKeyEvent *event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier, "test");
    selectview.keyPressEvent(event);
    delete event;
    event = new QKeyEvent(QEvent::KeyPress, Qt::Key_PageDown, Qt::NoModifier, "test");
    selectview.keyPressEvent(event);
    delete event;
    event = new QKeyEvent(QEvent::KeyPress, Qt::Key_PageUp, Qt::NoModifier, "test");
    selectview.keyPressEvent(event);
    delete event;
    event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Home, Qt::NoModifier, "test");
    selectview.keyPressEvent(event);
    delete event;
}

TEST_F(UT_FolderSelectView, UT_FolderSelectView_mouseMoveEvent_001)
{
    FolderSelectView selectview;
    QPointF localPos(30, 20);
    QMouseEvent *mouseMoveEvent = new QMouseEvent(QEvent::MouseMove, localPos, localPos, localPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSource::MouseEventSynthesizedByQt);
    selectview.mouseMoveEvent(mouseMoveEvent);
    EXPECT_EQ(Qt::NoModifier, mouseMoveEvent->modifiers());
    delete mouseMoveEvent;
}

TEST_F(UT_FolderSelectView, UT_FolderSelectView_mouseMoveEvent_002)
{
    FolderSelectView selectview;
    QPointF localPos(30, 20);
    QMouseEvent *mouseMoveEvent = new QMouseEvent(QEvent::MouseMove, localPos, localPos, localPos, Qt::LeftButton,
                                                  Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSource::MouseEventNotSynthesized);
    selectview.mouseMoveEvent(mouseMoveEvent);
    EXPECT_EQ(Qt::NoModifier, mouseMoveEvent->modifiers());
    delete mouseMoveEvent;
}

TEST_F(UT_FolderSelectView, UT_FolderSelectView_focusInEvent_001)
{
    FolderSelectView selectview;
    QStandardItemModel *pDataModel = new QStandardItemModel();
    LeftViewDelegate *pItemDelegate = new LeftViewDelegate(&selectview);
    selectview.setModel(pDataModel);
    selectview.setItemDelegate(pItemDelegate);
    QFocusEvent *mouseMoveEvent = new QFocusEvent(QEvent::MouseMove, Qt::TabFocusReason);
    selectview.focusInEvent(mouseMoveEvent);
    delete pDataModel;
    delete pItemDelegate;
    delete mouseMoveEvent;
}

TEST_F(UT_FolderSelectView, UT_FolderSelectView_focusInEvent_002)
{
    FolderSelectView selectview;
    QStandardItemModel *pDataModel = new QStandardItemModel();
    LeftViewDelegate *pItemDelegate = new LeftViewDelegate(&selectview);
    selectview.setModel(pDataModel);
    selectview.setItemDelegate(pItemDelegate);
    QFocusEvent *mouseMoveEvent = new QFocusEvent(QEvent::MouseMove);
    selectview.focusInEvent(mouseMoveEvent);
    delete pDataModel;
    delete pItemDelegate;
    delete mouseMoveEvent;
}

TEST_F(UT_FolderSelectView, UT_FolderSelectView_focusOutEvent_001)
{
    FolderSelectView selectview;
    QStandardItemModel *pDataModel = new QStandardItemModel();
    LeftViewDelegate *pItemDelegate = new LeftViewDelegate(&selectview);
    selectview.setModel(pDataModel);
    selectview.setItemDelegate(pItemDelegate);
    QFocusEvent *mouseMoveEvent = new QFocusEvent(QEvent::MouseMove, Qt::TabFocusReason);
    selectview.focusOutEvent(mouseMoveEvent);
    delete pDataModel;
    delete pItemDelegate;
    delete mouseMoveEvent;
}

TEST_F(UT_FolderSelectView, UT_FolderSelectView_doTouchMoveEvent_001)
{
    FolderSelectView selectview;
    QPointF localPos(30, 20);
    QMouseEvent *mouseMoveEvent = new QMouseEvent(QEvent::MouseMove, localPos, localPos, localPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSource::MouseEventSynthesizedByQt);
    selectview.m_isTouchSliding = false;
    selectview.doTouchMoveEvent(mouseMoveEvent);
    EXPECT_TRUE(selectview.m_isTouchSliding) << "m_isTouchSliding is false";
    selectview.doTouchMoveEvent(mouseMoveEvent);
    EXPECT_TRUE(selectview.m_isTouchSliding) << "m_isTouchSliding is true";
    delete mouseMoveEvent;
}

TEST_F(UT_FolderSelectView, UT_FolderSelectView_handleTouchSlideEvent_001)
{
    FolderSelectView selectview;
    QPoint localPos(30, 20);
    selectview.handleTouchSlideEvent(20, 40, localPos);
    EXPECT_EQ(20, selectview.m_touchPressPointY);
}
