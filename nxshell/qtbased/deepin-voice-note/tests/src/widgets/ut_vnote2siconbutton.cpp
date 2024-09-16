// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_vnote2siconbutton.h"
#include "vnote2siconbutton.h"
#include "utils.h"

UT_VNote2SIconButton::UT_VNote2SIconButton()
{
}

TEST_F(UT_VNote2SIconButton, isPressed)
{
    VNote2SIconButton vnote2siconbutton("test", "test");
    vnote2siconbutton.isPressed();
    vnote2siconbutton.setCommonIcon(true);
    EXPECT_EQ(vnote2siconbutton.m_useCommonIcons, true);
}

TEST_F(UT_VNote2SIconButton, UT_VNote2SIconButton_mouseReleaseEvent_001)
{
    VNote2SIconButton vnote2siconbutton("test", "test");
    QPointF localPos;
    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonPress, localPos, Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    int status = vnote2siconbutton.m_state;
    vnote2siconbutton.mouseReleaseEvent(event);
    EXPECT_TRUE(status != vnote2siconbutton.m_state);
    delete event;
}

TEST_F(UT_VNote2SIconButton, UT_VNote2SIconButton_keyPressEvent_001)
{
    VNote2SIconButton vnote2siconbutton("test", "test");
    int status = vnote2siconbutton.m_state;
    QKeyEvent e(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    vnote2siconbutton.keyPressEvent(&e);
    EXPECT_TRUE(status != vnote2siconbutton.m_state);
}
