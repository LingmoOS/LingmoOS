// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "gui/exhibition/preview/generalwidget/generaltoolbar.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <DGuiApplicationHelper>

#include <QFont>

using namespace testing;
DGUI_USE_NAMESPACE;
GRANDSEARCH_USE_NAMESPACE

TEST(GeneralToolBarTest, constructor)
{
    stub_ext::StubExt stu;

    stu.set_lamda(ADDR(QFont, pixelSize), [&](){
        return 12;
    });

    GeneralToolBar *w = new GeneralToolBar;

    ASSERT_TRUE(w);
    ASSERT_TRUE(w->m_openBtn);
    ASSERT_TRUE(w->m_openPathBtn);
    ASSERT_TRUE(w->m_copyPathBtn);

    Qt::ToolButtonStyle style = w->m_openBtn->toolButtonStyle();
    EXPECT_EQ(style, Qt::ToolButtonTextBesideIcon);

    delete w;
}

TEST(GeneralToolBarTest, constructor_max)
{
    stub_ext::StubExt stu;

    DGuiApplicationHelper::ColorType ut_themeType = DGuiApplicationHelper::DarkType;
    stu.set_lamda(ADDR(DGuiApplicationHelper, themeType), [&](){
        return ut_themeType;
    });

    stu.set_lamda(ADDR(QFont, pixelSize), [&](){
        return 20;
    });

    GeneralToolBar w;

    Qt::ToolButtonStyle style = w.m_openBtn->toolButtonStyle();
    EXPECT_EQ(style, Qt::ToolButtonIconOnly);
}
