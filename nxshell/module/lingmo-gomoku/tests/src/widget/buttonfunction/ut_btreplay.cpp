// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_btreplay.h"

TEST_F(UT_BTReplay, UT_BTReplay_boundingRect)
{
    m_ButtonReplay->boundingRect();
    EXPECT_EQ(m_ButtonReplay->boundingRect().width(), 204)
            << "check btreplay boundingRect";
}

TEST_F(UT_BTReplay, UT_BTReplay_paint1)
{
    QStyleOptionGraphicsItem *option = nullptr;
    QWidget *widget = nullptr;
    QPainter painter;
    m_ButtonReplay->firstStartGame = true;
    m_ButtonReplay->paint(&painter, option, widget);
}

TEST_F(UT_BTReplay, UT_BTReplay_paint2)
{
    QStyleOptionGraphicsItem *option = nullptr;
    QWidget *widget = nullptr;
    QPainter painter;
    m_ButtonReplay->firstStartGame = false;
    m_ButtonReplay->pressStatus = true;
    m_ButtonReplay->paint(&painter, option, widget);
}

TEST_F(UT_BTReplay, UT_BTReplay_setButtonState)
{
    m_ButtonReplay->setButtonState(true);
    EXPECT_EQ(m_ButtonReplay->firstStartGame, true)
            << "check btreplay setbuttonState";
}

TEST_F(UT_BTReplay, UT_BTReplay_buttonFunction)
{
    m_ButtonReplay->firstStartGame = false;
    m_ButtonReplay->buttonFunction();
}

TEST_F(UT_BTReplay, UT_BTReplay_setNotFirstGame)
{
    m_ButtonReplay->posHeight = 1.0;
    m_ButtonReplay->setNotFirstGame();
    EXPECT_EQ(m_ButtonReplay->firstStartGame, false)
            << "check btreplay setNotFirstGame";
}

TEST_F(UT_BTReplay, UT_BTReplay_getPosHeighe)
{
    m_ButtonReplay->posHeight = 1.0;
    qreal res = m_ButtonReplay->getPosHeight();
    EXPECT_EQ(res, 1.0)
            << "check btreplay getPosHeight";
}

TEST_F(UT_BTReplay, UT_BTReplay_getFirstGamePosHeight)
{
    const qreal res = m_ButtonReplay->getFirstGamePosHeight();
    EXPECT_EQ(res, 0.3425)
            << "check btreplay getFirstGamePosHeight";
}
