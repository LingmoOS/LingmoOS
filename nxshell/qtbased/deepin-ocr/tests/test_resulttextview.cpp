// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QTestEventList>
#include <QObject>
#include <QStandardPaths>
#include <QResizeEvent>
#include <QSize>
#include <QGesture>
#include <QTapAndHoldGesture>
#include <QMenu>

#define private public
#define protected public

#include "ocrapplication.h"
#include "mainwidget.h"
#include "view/imageview.h"
#include "resulttextview.h"
#include "stub.h"
#include "addr_pri.h"
//初始拉起主界面


Qt::GestureState state_stub(void* obj)
{
    Q_UNUSED(obj);
    return Qt::GestureStarted;
}
Qt::GestureState state_stub1(void* obj)
{
    Q_UNUSED(obj);
    return Qt::GestureUpdated;
}
Qt::GestureState state_stub2(void* obj)
{
    Q_UNUSED(obj);
    return Qt::GestureCanceled;
}
Qt::GestureState state_stub3(void* obj)
{
    Q_UNUSED(obj);
    return Qt::GestureFinished;
}

QAction *exec_stub(void* obj, const QPoint &pos, QAction *at)
{
    Q_UNUSED(obj);
    Q_UNUSED(pos);
    Q_UNUSED(at);
    return nullptr;
}
TEST(ResultTextView, mainwindow)
{
    {
        ResultTextView* reTextView = new ResultTextView(nullptr);
        QResizeEvent resizeEvent(QSize(500, 200), QSize(500, 400));
        reTextView->resizeEvent(&resizeEvent);

        QMouseEvent *ev = new QMouseEvent(QEvent::MouseMove, QPoint(15,36), QPoint(25,40), QPoint(60,80), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
        reTextView->m_gestureAction = ResultTextView::GA_slide;
        reTextView->mouseMoveEvent(ev);
        delete ev;

        Stub stub;

        stub.set((QAction*(QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), exec_stub);
        reTextView->contextMenuEvent(nullptr);
        stub.reset((QAction*(QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec));

        QMouseEvent *evPress = new QMouseEvent(QEvent::MouseButtonPress, QPoint(15,36), QPoint(25,40), QPoint(60,80), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
        reTextView->mousePressEvent(evPress);
        delete evPress;

        QMouseEvent *evRele = new QMouseEvent(QEvent::MouseButtonRelease, QPoint(15,36), QPoint(25,40), QPoint(60,80), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
        reTextView->mouseReleaseEvent(evRele);
        delete evRele;

        QList<QGesture*> list;
        QTapGesture* tap = new QTapGesture();
        list.append(tap);
        QGestureEvent *gestureEvent = new QGestureEvent(list);
        reTextView->event(gestureEvent);
        stub.set(ADDR(QTapGesture,state), state_stub);
        reTextView->event(gestureEvent);
        stub.reset(ADDR(QTapGesture,state));
        stub.set(ADDR(QTapGesture,state), state_stub1);
        reTextView->event(gestureEvent);
        stub.reset(ADDR(QTapGesture,state));
        stub.set(ADDR(QTapGesture,state), state_stub2);
        reTextView->event(gestureEvent);
        stub.reset(ADDR(QTapGesture,state));
        stub.set(ADDR(QTapGesture,state), state_stub3);
        reTextView->event(gestureEvent);
        stub.reset(ADDR(QTapGesture,state));

        delete gestureEvent;
        delete tap;
        delete reTextView;
    }

    {
        ResultTextView* reTextView = new ResultTextView(nullptr);
        QResizeEvent resizeEvent(QSize(500, 200), QSize(500, 400));
        reTextView->resizeEvent(&resizeEvent);

        Stub stub;

        QList<QGesture*> list1;
        QTapAndHoldGesture* tapAndHold = new QTapAndHoldGesture();
        list1.append(tapAndHold);
        QGestureEvent *gestureEvent1 = new QGestureEvent(list1);
        reTextView->event(gestureEvent1);
        stub.set(ADDR(QTapAndHoldGesture,state), state_stub);
        reTextView->event(gestureEvent1);
        stub.reset(ADDR(QTapAndHoldGesture,state));
        stub.set(ADDR(QTapAndHoldGesture,state), state_stub1);
        reTextView->event(gestureEvent1);
        stub.reset(ADDR(QTapAndHoldGesture,state));
        stub.set(ADDR(QTapAndHoldGesture,state), state_stub2);
        reTextView->event(gestureEvent1);
        stub.reset(ADDR(QTapAndHoldGesture,state));
        stub.set(ADDR(QTapAndHoldGesture,state), state_stub3);
        reTextView->event(gestureEvent1);
        stub.reset(ADDR(QTapAndHoldGesture,state));

        delete gestureEvent1;
        delete tapAndHold;
        delete reTextView;
    }

    {
        ResultTextView* reTextView = new ResultTextView(nullptr);
        QResizeEvent resizeEvent(QSize(500, 200), QSize(500, 400));
        reTextView->resizeEvent(&resizeEvent);

        Stub stub;

        QList<QGesture*> list2;
        QPanGesture* pan = new QPanGesture();
        list2.append(pan);
        QGestureEvent *gestureEvent2 = new QGestureEvent(list2);
        reTextView->event(gestureEvent2);
        stub.set(ADDR(QPanGesture,state), state_stub);
        reTextView->event(gestureEvent2);
        stub.reset(ADDR(QPanGesture,state));
        stub.set(ADDR(QPanGesture,state), state_stub1);
        reTextView->event(gestureEvent2);
        stub.reset(ADDR(QPanGesture,state));
        stub.set(ADDR(QPanGesture,state), state_stub2);
        reTextView->event(gestureEvent2);
        stub.reset(ADDR(QPanGesture,state));
        stub.set(ADDR(QPanGesture,state), state_stub3);
        reTextView->event(gestureEvent2);
        stub.reset(ADDR(QPanGesture,state));

        delete gestureEvent2;
        delete pan;
        delete reTextView;
    }

    {
        ResultTextView* reTextView = new ResultTextView(nullptr);
        QResizeEvent resizeEvent(QSize(500, 200), QSize(500, 400));
        reTextView->resizeEvent(&resizeEvent);

        Stub stub;

        QList<QGesture*> list3;
        QPinchGesture* pinch = new QPinchGesture();
        list3.append(pinch);
        QGestureEvent *gestureEvent3 = new QGestureEvent(list3);
        reTextView->event(gestureEvent3);
        stub.set(ADDR(QPinchGesture,state), state_stub);
        reTextView->event(gestureEvent3);
        stub.reset(ADDR(QPinchGesture,state));
        stub.set(ADDR(QPinchGesture,state), state_stub1);
        reTextView->event(gestureEvent3);
        stub.reset(ADDR(QPinchGesture,state));
        stub.set(ADDR(QPinchGesture,state), state_stub2);
        reTextView->event(gestureEvent3);
        stub.reset(ADDR(QPinchGesture,state));
        stub.set(ADDR(QPinchGesture,state), state_stub3);
        reTextView->event(gestureEvent3);
        stub.reset(ADDR(QPinchGesture,state));
        delete gestureEvent3;
        delete pinch;

        reTextView->slideGestureY(2.5);
        reTextView->slideGestureX(2.5);

        delete reTextView;
    }
}
