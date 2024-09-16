// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "stubext.h"

#include <gtest/gtest.h>

#include <QTimer>
#include <QtTest>

#define private public
#define protected public
#include "gui/mainwindow.h"
#include "gui/handlevisibility/handlevisibility.h"
#include "dregionmonitor.h"

using namespace testing;
DGUI_USE_NAMESPACE;
GRANDSEARCH_USE_NAMESPACE

TEST(HandleVisibilityTest, constructor)
{
    MainWindow w;
    HandleVisibility hand(&w, nullptr);

    ASSERT_TRUE(hand.m_mainWindow);
    ASSERT_TRUE(hand.m_regionMonitor);
}

TEST(HandleVisibilityTest, onApplicationStateChanged)
{
    MainWindow w;
    HandleVisibility hand(&w, nullptr);

    stub_ext::StubExt stu;

    bool ut_show = false;

    stu.set_lamda(ADDR(QWidget, show), [&](){
        ut_show = true;
    });

    hand.onApplicationStateChanged(Qt::ApplicationInactive);
    EXPECT_FALSE(ut_show);

    ut_show = false;
    hand.onApplicationStateChanged(Qt::ApplicationActive);
    EXPECT_TRUE(ut_show);
}

TEST(HandleVisibilityTest, onLockedChanged)
{
    MainWindow w;
    HandleVisibility hand(&w, nullptr);

    stub_ext::StubExt stu;

    bool visible = false;
    typedef void (*fptr)(QWidget*,bool);
    fptr ut_setVisible = (fptr)(&QWidget::setVisible);
    stu.set_lamda(ut_setVisible, [&](QWidget *obj, bool isVisible){
        visible = isVisible;
    });

    hand.onLockedChanged(false);
    EXPECT_TRUE(visible);

    visible = true;
    hand.onLockedChanged(true);
    EXPECT_FALSE(visible);
}

TEST(HandleVisibilityTest, onCloseWindow)
{
    MainWindow w;
    HandleVisibility hand(&w, nullptr);

    hand.onCloseWindow();
    EXPECT_FALSE(hand.m_mainWindow->isVisible());
}

TEST(HandleVisibilityTest, registerRegion)
{
    MainWindow w;
    HandleVisibility hand(&w, nullptr);

    stub_ext::StubExt stu;

    bool ut_get_register = false;
    bool ut_set_register = false;
    bool ut_set_unregister = false;

    stu.set_lamda(ADDR(DRegionMonitor, registered), [&](){
       return ut_get_register;
    });

    stu.set_lamda(((void(DRegionMonitor::*)(void))ADDR(DRegionMonitor, registerRegion)), [&](){
        ut_set_register = true;
    });

    stu.set_lamda(ADDR(DRegionMonitor, unregisterRegion), [&](){
        ut_set_unregister = true;
    });

    hand.registerRegion(false);
    EXPECT_FALSE(ut_set_register);
    EXPECT_FALSE(ut_set_unregister);

    ut_get_register = false;
    ut_set_register = false;
    ut_set_unregister = false;
    hand.registerRegion(true);
    EXPECT_TRUE(ut_set_register);
    EXPECT_FALSE(ut_set_unregister);

    ut_get_register = true;
    ut_set_register = false;
    ut_set_unregister = false;
    hand.registerRegion(false);
    EXPECT_FALSE(ut_set_register);
    EXPECT_TRUE(ut_set_unregister);
}

TEST(HandleVisibilityTest, regionMousePress)
{
    MainWindow w;
    w.setGeometry(100, 100, 100, 100);

    HandleVisibility hand(&w, nullptr);

    bool isCall = false;
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(HandleVisibility, onCloseWindow), [&](){
       isCall = true;
    });

    hand.m_regionMonitor->buttonPress(QPoint(-1, -1), 0);

    QTest::qWaitFor([&](){
        return isCall;
    }, 2000);

    EXPECT_TRUE(isCall);
}
