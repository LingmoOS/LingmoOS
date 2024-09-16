// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "gui/exhibition/matchresult/viewmore/viewmorebutton.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <DGuiApplicationHelper>
#include <DStyle>

#include <QPaintEvent>
#include <QStyleOptionToolButton>

GRANDSEARCH_USE_NAMESPACE
DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

TEST(ViewMoreButtonTest, setSelected)
{
    ViewMoreButton btn("test");
    btn.setSelected(false);
    EXPECT_FALSE(btn.m_bIsSelected);

    stub_ext::StubExt st;
    st.set_lamda((void(ViewMoreButton::*)())ADDR(ViewMoreButton, update), [](){
        return ;
    });
    btn.setSelected(true);
    EXPECT_TRUE(btn.m_bIsSelected);
}

TEST(ViewMoreButtonTest, isSelected)
{
    ViewMoreButton btn("test");
    bool selected = btn.isSelected();
    EXPECT_EQ(btn.m_bIsSelected, selected);
}

TEST(ViewMoreButtonTest, paintEvent)
{
    ViewMoreButton btn("test");
    QPaintEvent event((QRect()));

    stub_ext::StubExt st;
    st.set_lamda(&DGuiApplicationHelper::themeType, [&](){
        return DGuiApplicationHelper::DarkType;
    });

    st.set_lamda(&ViewMoreButton::initStyleOption, [](QToolButton *, QStyleOptionToolButton *opt){
        opt->state |= QStyle::State_Active;
    });

    // normal, not selected
    st.set_lamda(&DStyle::getState, [](){ return DStyle::SS_NormalState; });
    EXPECT_NO_FATAL_FAILURE(btn.paintEvent(&event));
    // normal, selected
    btn.m_bIsSelected = true;
    EXPECT_NO_FATAL_FAILURE(btn.paintEvent(&event));

    // hover
    st.reset(&DStyle::getState);
    st.set_lamda(&DStyle::getState, [](){ return DStyle::SS_HoverState; });
    EXPECT_NO_FATAL_FAILURE(btn.paintEvent(&event));

    // press
    st.reset(&DStyle::getState);
    st.set_lamda(&DStyle::getState, [](){ return DStyle::SS_PressState; });
    EXPECT_NO_FATAL_FAILURE(btn.paintEvent(&event));
}
