// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "contacts/services/grandsearchservice.h"
#include "gui/mainwindow.h"

#include "services/grandsearchserviceadaptor.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;
GRANDSEARCH_USE_NAMESPACE

TEST(GrandSearchServiceAdaptorTest, constructor)
{
    MainWindow w;
    GrandSearchService service(&w);
    GrandSearchServiceAdaptor *adaptor = new GrandSearchServiceAdaptor(&service);

    ASSERT_TRUE(adaptor);

    delete adaptor;
}

TEST(GrandSearchServiceAdaptorTest, IsVisible)
{
    MainWindow w;
    GrandSearchService service(&w);
    GrandSearchServiceAdaptor adaptor(&service);

    stub_ext::StubExt stu;
    bool isCall = false;
    stu.set_lamda(ADDR(GrandSearchService, IsVisible), [&](){
        isCall = true;
        return isCall;
    });

    adaptor.IsVisible();
    EXPECT_TRUE(isCall);
}

TEST(GrandSearchServiceAdaptorTest, SetVisible)
{
    MainWindow w;
    GrandSearchService service(&w);
    GrandSearchServiceAdaptor adaptor(&service);

    stub_ext::StubExt stu;
    bool isCall = false;
    stu.set_lamda(ADDR(GrandSearchService, SetVisible), [&](){
        isCall = true;
    });

    adaptor.SetVisible(isCall);
    EXPECT_TRUE(isCall);
}
