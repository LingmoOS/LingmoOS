// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_btmusiccontrol.h"

TEST_F(UT_BTMusicControl, UT_BTMusicControl_boundingRect)
{
    m_ButtonMusicControl->boundingRect();
    EXPECT_EQ(m_ButtonMusicControl->boundingRect().width(), 204)
            << "check btreplay boundingRect";
}

TEST_F(UT_BTMusicControl, UT_BTMusicControl_paint1)
{
    QStyleOptionGraphicsItem *option = nullptr;
    QWidget *widget = nullptr;
    QPainter painter;
    m_ButtonMusicControl->mouseReleased = true;
    m_ButtonMusicControl->pressStatus = true;
    m_ButtonMusicControl->paint(&painter, option, widget);
}

TEST_F(UT_BTMusicControl, UT_BTMusicControl_paint2)
{
    QStyleOptionGraphicsItem *option = nullptr;
    QWidget *widget = nullptr;
    QPainter painter;
    m_ButtonMusicControl->mouseReleased = false;
    m_ButtonMusicControl->pressStatus = false;
    m_ButtonMusicControl->paint(&painter, option, widget);
}

TEST_F(UT_BTMusicControl, UT_BTMusicControl_buttonFunction)
{
    m_ButtonMusicControl->buttonFunction();
}

TEST_F(UT_BTMusicControl, UT_BTMusicControl_setNotFirstGame)
{
    m_ButtonMusicControl->setNotFirstGame();
}
