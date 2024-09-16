// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_vnoterightmenu.h"
#include "vnoterightmenu.h"
#include "globaldef.h"

UT_VNoteRightMenu::UT_VNoteRightMenu()
{
}

TEST_F(UT_VNoteRightMenu, UT_VNoteRightMenu_mouseMoveEvent_001)
{
    VNoteRightMenu rightmenu;
    QPointF localPos;
    QMouseEvent *mouseMoveEvent = new QMouseEvent(QEvent::MouseMove, localPos, localPos, localPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSource::MouseEventSynthesizedByQt);
    rightmenu.m_timer->start(100);
    rightmenu.m_moved = true;
    rightmenu.mouseMoveEvent(mouseMoveEvent);
    EXPECT_FALSE(rightmenu.m_timer->isActive());
    delete mouseMoveEvent;
}

TEST_F(UT_VNoteRightMenu, UT_VNoteRightMenu_setPressPoint_001)
{
    VNoteRightMenu rightmenu;
    QPoint pos(10, 10);
    rightmenu.setPressPoint(pos);
    EXPECT_EQ(rightmenu.m_touchPoint, pos);
}
