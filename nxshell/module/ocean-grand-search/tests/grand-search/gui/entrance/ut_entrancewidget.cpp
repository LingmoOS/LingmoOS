// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "stubext.h"

#include <gtest/gtest.h>

#include <QEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QContextMenuEvent>
#include <QMenu>

#define private public
#define protected public
#include "gui/entrance/entrancewidget.h"
#include "gui/entrance/entrancewidget_p.h"
#include "utils/utils.h"

using namespace testing;
GRANDSEARCH_USE_NAMESPACE

TEST(EntranceWidgetTest, constructor)
{
    EntranceWidget *w = new EntranceWidget;

    ASSERT_TRUE(w->d_p);
    EXPECT_TRUE(w->d_p->m_searchEdit);
    EXPECT_TRUE(w->d_p->m_lineEdit);
    EXPECT_TRUE(w->d_p->m_appIconLabel);
    EXPECT_TRUE(w->d_p->m_appIconAction);

    delete w;
}

TEST(EntranceWidgetPrivate, delayChangeText)
{
    EntranceWidget w;
    ASSERT_TRUE(w.d_p);

    w.d_p->delayChangeText();
    EXPECT_TRUE(w.d_p->m_delayChangeTimer->isActive());
    w.d_p->m_delayChangeTimer->stop();
}

TEST(EntranceWidgetPrivate, notifyTextChanged)
{
    EntranceWidget w;
    ASSERT_TRUE(w.d_p);

    stub_ext::StubExt stu;

    bool ut_call = false;

    stu.set_lamda(ADDR(EntranceWidget, onAppIconChanged), [&](){
        ut_call = true;
    });

    w.d_p->notifyTextChanged();
    EXPECT_TRUE(ut_call);
}

TEST(EntranceWidgetPrivate, showMenu)
{
    EntranceWidget w;
    ASSERT_TRUE(w.d_p);

    stub_ext::StubExt stu;

    bool ut_call = false;

    stu.set_lamda(((QAction*(QMenu::*)(const QPoint&,QAction*))ADDR(QMenu, exec)), [&](){
        ut_call = true;
        return nullptr;
    });

    w.d_p->showMenu(QPoint());
    EXPECT_TRUE(ut_call);
}

TEST(EntranceWidgetTest, showLabelAppIcon)
{
    stub_ext::StubExt stu;

    bool ut_get_visible = false;

    stu.set_lamda(ADDR(QWidget, isVisible), [&](){
        return ut_get_visible;
    });

    bool ut_set_visible = true;

    typedef void (*fptr)(QWidget*,bool);
    fptr ut_setVisible = (fptr)(&QWidget::setVisible);
    stu.set_lamda(ut_setVisible, [&](QWidget *obj, bool isVisible){
        ut_set_visible = isVisible;
    });

    EntranceWidget w;

    w.showLabelAppIcon(false);
    EXPECT_TRUE(ut_set_visible);

    ut_set_visible = false;
    w.showLabelAppIcon(true);
    EXPECT_TRUE(ut_set_visible);
}

TEST(EntranceWidgetTest, event)
{
    stub_ext::StubExt stu;

    bool isCall = false;

    stu.set_lamda(((void(QWidget::*)(void))ADDR(QWidget, setFocus)), [&](){
        isCall = true;
    });

    QEvent eventIn(QEvent::FocusIn);

    EntranceWidget w;
    w.event(&eventIn);
    EXPECT_TRUE(isCall);

    isCall = false;
    QEvent eventOut(QEvent::FocusOut);
    w.event(&eventOut);
    EXPECT_FALSE(isCall);
}

TEST(EntranceWidgetTest, paintEvent)
{
    bool isCall = false;

    stub_ext::StubExt stu;
    typedef void (*fptr)(QFrame*,QPaintEvent*);
    fptr ut_paintEvent = (fptr)(&QFrame::paintEvent);
    stu.set_lamda(ut_paintEvent, [&](){
        isCall = true;
    });

    QPaintEvent event(QRect(0, 0, 100, 100));

    EntranceWidget w;
    w.paintEvent(&event);
    EXPECT_TRUE(isCall);
}

TEST(EntranceWidgetTest, eventFilter)
{
    EntranceWidget w;
    ASSERT_TRUE(w.d_p->m_lineEdit);

    bool isCallQFrameEventFilter = false;

    stub_ext::StubExt stu;
    typedef bool (*fptr)(QFrame*,QObject *, QEvent *);
    fptr ut_eventFilter = (fptr)(&QFrame::eventFilter);
    stu.set_lamda(ut_eventFilter, [&](){
        isCallQFrameEventFilter = true;
        return true;
    });

    QKeyEvent keyUpEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    bool result = w.eventFilter((QObject*)(w.d_p->m_lineEdit), &keyUpEvent);
    EXPECT_TRUE(result);
    EXPECT_FALSE(isCallQFrameEventFilter);

    isCallQFrameEventFilter = false;
    QKeyEvent keyTabEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    result = w.eventFilter((QObject*)(w.d_p->m_lineEdit), &keyTabEvent);
    EXPECT_TRUE(result);
    EXPECT_FALSE(isCallQFrameEventFilter);

    isCallQFrameEventFilter = false;
    QKeyEvent keyDownEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    result = w.eventFilter((QObject*)(w.d_p->m_lineEdit), &keyDownEvent);
    EXPECT_TRUE(result);
    EXPECT_FALSE(isCallQFrameEventFilter);

    isCallQFrameEventFilter = false;
    QKeyEvent keyEnterEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    result = w.eventFilter((QObject*)(w.d_p->m_lineEdit), &keyEnterEvent);
    EXPECT_TRUE(result);
    EXPECT_FALSE(isCallQFrameEventFilter);

    isCallQFrameEventFilter = false;
    QKeyEvent keyReturnEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    result = w.eventFilter((QObject*)(w.d_p->m_lineEdit), &keyReturnEvent);
    EXPECT_TRUE(result);
    EXPECT_FALSE(isCallQFrameEventFilter);

    isCallQFrameEventFilter = false;
    QKeyEvent keyEscapeEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    result = w.eventFilter((QObject*)(w.d_p->m_lineEdit), &keyEscapeEvent);
    EXPECT_TRUE(result);
    EXPECT_FALSE(isCallQFrameEventFilter);

    isCallQFrameEventFilter = false;
    QKeyEvent keyAEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    result = w.eventFilter((QObject*)(w.d_p->m_lineEdit), &keyAEvent);
    EXPECT_TRUE(result);
    EXPECT_TRUE(isCallQFrameEventFilter);

    bool isCallShowMenu = false;
    stu.set_lamda(ADDR(EntranceWidgetPrivate, showMenu), [&](){
        isCallShowMenu = true;
    });
    isCallQFrameEventFilter = false;
    QContextMenuEvent menuEvent(QContextMenuEvent::Other, QPoint(0, 0));
    result = w.eventFilter((QObject*)(w.d_p->m_lineEdit), &menuEvent);
    EXPECT_FALSE(isCallQFrameEventFilter);
    EXPECT_TRUE(isCallShowMenu);
    EXPECT_TRUE(result);

    bool isCallSetFocus = false;
    stu.set_lamda(((void(QWidget::*)(void))ADDR(QWidget, setFocus)), [&](){
        isCallSetFocus = true;
    });
    isCallQFrameEventFilter = false;
    isCallShowMenu = false;
    QEvent eventFocusIn(QEvent::FocusIn);
    result = w.eventFilter((QObject*)(w.d_p->m_searchEdit), &eventFocusIn);
    EXPECT_FALSE(isCallQFrameEventFilter);
    EXPECT_FALSE(isCallShowMenu);
    EXPECT_TRUE(isCallSetFocus);
    EXPECT_TRUE(result);


    isCallQFrameEventFilter = false;
    isCallShowMenu = false;
    isCallSetFocus = false;
    QEvent otherEvent(QEvent::FocusOut);
    result = w.eventFilter((QObject*)(w.d_p->m_searchEdit), &otherEvent);
    EXPECT_TRUE(isCallQFrameEventFilter);
    EXPECT_FALSE(isCallShowMenu);
    EXPECT_FALSE(isCallSetFocus);
    EXPECT_TRUE(result);
}

TEST(EntranceWidgetTest, onAppIconChanged)
{
    bool ut_setValue = false;

    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(EntranceWidget, showLabelAppIcon), [&](EntranceWidget *obj, bool isShow){
        Q_UNUSED(obj);

        ut_setValue = isShow;
    });

    QString ut_name;
    stu.set_lamda(ADDR(Utils, appIconName), [&](){
        return ut_name;
    });

    QString searchGroupName;
    GrandSearch::MatchedItem item;

    EntranceWidget w;

    ut_name.clear();
    ut_setValue = true;
    w.onAppIconChanged(searchGroupName, item);
    EXPECT_FALSE(ut_setValue);
    EXPECT_TRUE(w.d_p->m_appIconName.isEmpty());

    ut_name = QString("test");
    w.d_p->m_appIconName = ut_name;
    ut_setValue = false;
    w.onAppIconChanged(searchGroupName, item);

    w.d_p->m_appIconName.clear();
    w.onAppIconChanged(searchGroupName, item);
    EXPECT_TRUE(ut_setValue);
    EXPECT_EQ(ut_name, w.d_p->m_appIconName);
}
