// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QGuiApplication>
#include <QMouseEvent>
#include <QCloseEvent>

#define private public
#define protected public
#include "gui/mainwindow.h"
#include "gui/mainwindow_p.h"
#include "gui/entrance/entrancewidget.h"
#include "gui/handlevisibility/handlevisibility.h"

using namespace testing;
GRANDSEARCH_USE_NAMESPACE

TEST(MainWindowTest, constructor)
{
    MainWindow *mw = new MainWindow;

    EXPECT_TRUE(mw->d_p);
    EXPECT_TRUE(mw->d_p->m_entranceWidget);
    EXPECT_TRUE(mw->d_p->m_exhibitionWidget);
    EXPECT_TRUE(mw->d_p->m_searchNoContentWidget);

    delete mw;
}

TEST(MainWindowTest, connectToController)
{
    MainWindow w;
    w.connectToController();

    ASSERT_TRUE(w.d_p);
    EXPECT_TRUE(w.d_p->m_queryController);
    EXPECT_TRUE(w.d_p->m_matchController);
}

TEST(MainWindowTest, onPrimaryScreenChanged)
{
    stub_ext::StubExt stu;

    bool isCall = false;

    stu.set_lamda(ADDR(MainWindow, onGeometryChanged), [&](){
        isCall = true;
    });

    MainWindow w;
    w.onPrimaryScreenChanged(qApp->primaryScreen());

    EXPECT_TRUE(isCall);
}

TEST(MainWindowTest, onGeometryChanged)
{
    QRect rect(0, 0, 760, 0);

    MainWindow w;
    w.onGeometryChanged(rect);

    QPoint point = w.geometry().topLeft();

    EXPECT_EQ(point, rect.topLeft());
}

TEST(MainWindowTest, onHideExhitionWidget)
{
    stub_ext::StubExt stu;

    bool isCall = false;

    stu.set_lamda(ADDR(MainWindow, updateMainWindowHeight), [&](){
        isCall = true;
    });

    MainWindow w;
    w.onHideExhitionWidget();

    EXPECT_TRUE(isCall);
}

TEST(MainWindowTest, onResetExhitionWidget)
{
    stub_ext::StubExt stu;

    bool isCall = false;

    stu.set_lamda(ADDR(MainWindow, updateMainWindowHeight), [&](){
        isCall = true;
    });

    MainWindow w;
    w.onResetExhitionWidget(QString());

    EXPECT_TRUE(isCall);
}

TEST(MainWindowTest, activeMainWindow)
{
    stub_ext::StubExt stu;

    bool isCall = false;

    stu.set_lamda(ADDR(QWidget, activateWindow), [&](){
        isCall = true;
    });

    MainWindow w;
    w.activeMainWindow();

    EXPECT_TRUE(isCall);
}

TEST(MainWindowTest, updateMainWindowHeight)
{
    stub_ext::StubExt stu;

    bool hide = false;

    stu.set_lamda(ADDR(QWidget, isHidden), [&](){
        return hide;
    });

    MainWindow w;
    w.updateMainWindowHeight();
    int hight = w.height();
    EXPECT_EQ(hight, 520);

    hide = true;
    w.updateMainWindowHeight();
    hight = w.height();
    EXPECT_EQ(hight, 68);
}

TEST(MainWindowTest, showExhitionWidget)
{
    stub_ext::StubExt stu;

    bool hide = true;

    stu.set_lamda(ADDR(QWidget, isHidden), [&](){
        return hide;
    });

    bool visible = true;

    typedef void (*fptr)(QWidget*,bool);
    fptr ut_setVisible = (fptr)(&QWidget::setVisible);
    stu.set_lamda(ut_setVisible, [&](QWidget *obj, bool isVisible){
        visible = isVisible;
    });

    MainWindow w;
    w.showExhitionWidget(false);

    w.showExhitionWidget(true);
    EXPECT_TRUE(visible);
}

TEST(MainWindowTest, showSerachNoContent)
{
    stub_ext::StubExt stu;

    bool isCall = false;

    stu.set_lamda(ADDR(MainWindow, showEntranceAppIcon), [&](){
        isCall = true;
    });

    MainWindow w;

    w.showSerachNoContent(false);
    EXPECT_FALSE(isCall);

    w.showSerachNoContent(true);
    EXPECT_TRUE(isCall);
}

TEST(MainWindowTest, showEntranceAppIcon)
{
    stub_ext::StubExt stu;

    bool value = false;

    stu.set_lamda(ADDR(EntranceWidget, showLabelAppIcon), [&](EntranceWidget *obj, bool show){
        Q_UNUSED(obj);

        value = show;
    });

    MainWindow w;
    w.showEntranceAppIcon(false);
    EXPECT_FALSE(value);

    w.showEntranceAppIcon(true);
    EXPECT_TRUE(value);
}

TEST(MainWindowTest, onFocusObjectChanged)
{
    stub_ext::StubExt stu;

    bool isCall = false;

    stu.set_lamda(((void(QWidget::*)(void))ADDR(QWidget, setFocus)), [&](){
        isCall = true;
    });

    MainWindow w;
    w.onFocusObjectChanged(&w);

    EXPECT_TRUE(isCall);
}

TEST(MainWindowTest, mousePressEvent)
{
    stub_ext::StubExt stu;

    bool isCall = false;

    stu.set_lamda(ADDR(MainWindow, activeMainWindow), [&](){
        isCall = true;
    });

    QMouseEvent event(QEvent::MouseButtonPress, QPointF(), QPointF(), QPointF(), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier, Qt::MouseEventSynthesizedByQt);

    MainWindow w;
    w.mousePressEvent(&event);

    EXPECT_TRUE(isCall);
}

TEST(MainWindowTest, showEvent)
{
    stub_ext::StubExt stu;

    bool value = false;

    stu.set_lamda(ADDR(HandleVisibility, registerRegion), [&](HandleVisibility *obj, bool isRegister){
        Q_UNUSED(obj);

        value = isRegister;
    });

    MainWindow w;
    w.show();

    EXPECT_TRUE(value);

}

TEST(MainWindowTest, hideEvent)
{
    stub_ext::StubExt stu;

    bool value = true;

    stu.set_lamda(ADDR(HandleVisibility, registerRegion), [&](HandleVisibility *obj, bool isRegister){
        Q_UNUSED(obj);

        value = isRegister;
    });

    MainWindow w;

    QHideEvent event;
    w.hideEvent(&event);

    EXPECT_FALSE(value);
}

TEST(MainWindowTest, closeEvent)
{
    MainWindow w;
    w.close();

    EXPECT_TRUE(w.isHidden());
}
