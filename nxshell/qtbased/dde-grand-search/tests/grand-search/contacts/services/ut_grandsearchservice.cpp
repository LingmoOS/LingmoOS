// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "gui/mainwindow.h"

#include "stubext.h"

#include <gtest/gtest.h>

#define private public
#define protected public
#include "contacts/services/grandsearchservice.h"
#include "contacts/services/grandsearchservice_p.h"

using namespace testing;
GRANDSEARCH_USE_NAMESPACE

TEST(GrandSearchServiceTest, constructor)
{
    MainWindow w;
    GrandSearchService *service = new GrandSearchService(&w);

    ASSERT_TRUE(service);
    ASSERT_TRUE(service->d_p);
    EXPECT_EQ(service->d_p->m_mainWindow, &w);

    delete service;
}

TEST(GrandSearchServiceTest, IsVisible)
{
    MainWindow w;
    GrandSearchService service(&w);

    bool actual = service.IsVisible();

    EXPECT_EQ(actual, service.d_p->m_mainWindow->isVisible());
}

TEST(GrandSearchServiceTest, SetVisible)
{
    MainWindow w;
    GrandSearchService service(&w);

    // 测试状态相等直接返回
    bool visible = service.d_p->m_mainWindow->isVisible();
    service.SetVisible(visible);

    // 测试状态不等进行设置
    stub_ext::StubExt stu;

    bool isCallClose = false;
    stu.set_lamda(ADDR(QWidget, close), [&](){
        isCallClose = true;
        return isCallClose;
    });

    bool testVisible = false;
    stu.set_lamda(ADDR(QWidget, isVisible), [&](){
        return testVisible;
    });

    typedef void (*fptr)(QWidget*, bool);
    fptr QWidget_setVisible = (fptr)(&QWidget::setVisible);

    bool isCallSetVisible = false;
    stu.set_lamda(QWidget_setVisible, [&](){
        isCallSetVisible = true;
    });

    // 设置为显示
    visible = true;
    service.SetVisible(visible);
    EXPECT_FALSE(isCallClose);
    EXPECT_TRUE(isCallSetVisible);

    // 设置为隐藏
    testVisible =true;
    visible = false;
    isCallSetVisible = false;
    service.SetVisible(visible);
    EXPECT_TRUE(isCallClose);
    EXPECT_FALSE(isCallSetVisible);
}
