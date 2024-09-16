// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_vnoteiconbutton.h"
#include "vnoteiconbutton.h"
#include "globaldef.h"

UT_VNoteIconButton::UT_VNoteIconButton()
{
}

TEST_F(UT_VNoteIconButton, UT_VNoteIconButton_mousePressEvent_001)
{
    VNoteIconButton vnoteiconbutton(nullptr, "home_page_logo.svg", "home_page_logo.svg", "home_page_logo.svg");
    QPointF localPos;
    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonPress, localPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    vnoteiconbutton.mouseMoveEvent(event);
    EXPECT_EQ(vnoteiconbutton.m_state, VNoteIconButton::Hover);
    vnoteiconbutton.mousePressEvent(event);
    EXPECT_EQ(vnoteiconbutton.m_state, VNoteIconButton::Press);
    vnoteiconbutton.mouseReleaseEvent(event);
    EXPECT_EQ(vnoteiconbutton.m_state, VNoteIconButton::Hover);
    delete event;
}

TEST_F(UT_VNoteIconButton, UT_VNoteIconButton_setSeparateThemIcons_001)
{
    VNoteIconButton vnoteiconbutton(nullptr, "home_page_logo.svg", "home_page_logo.svg", "home_page_logo.svg");
    vnoteiconbutton.enterEvent(nullptr);
    EXPECT_EQ(vnoteiconbutton.m_state, VNoteIconButton::Hover);
    vnoteiconbutton.leaveEvent(nullptr);
    EXPECT_EQ(vnoteiconbutton.m_state, VNoteIconButton::Normal);
    vnoteiconbutton.setSeparateThemIcons(false);
    EXPECT_EQ(vnoteiconbutton.m_separateThemeIcon, false);
    vnoteiconbutton.SetDisableIcon("home_page_logo.svg");
    EXPECT_FALSE(vnoteiconbutton.m_icons[VNoteIconButton::Disabled].isEmpty());
    vnoteiconbutton.setBtnDisabled(false);
    EXPECT_EQ(vnoteiconbutton.m_isDisabled, false);
    vnoteiconbutton.setBtnDisabled(true);
    EXPECT_EQ(vnoteiconbutton.m_isDisabled, true);
    EXPECT_EQ(vnoteiconbutton.getFocusReason(), Qt::NoFocusReason);
    vnoteiconbutton.onThemeChanged(DGuiApplicationHelper::LightType);
}
