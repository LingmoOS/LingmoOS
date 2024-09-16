// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_playingscreen.h"

#include <QPainter>

TEST_F(UT_PlayingScreen, UT_PlayingScreen_setCurrentChessColor)
{
    m_PlayingScreen->setCurrentChessColor(false, chess_black);
    EXPECT_EQ(m_PlayingScreen->AIPlayer, false)
            << "check playingScreen setCurrentChessColor";
}

TEST_F(UT_PlayingScreen, UT_PlayingScreen_setGameOverStatus)
{
    m_PlayingScreen->setGameOverStatus();
    EXPECT_EQ(m_PlayingScreen->gameOverStatus, true)
            << "check playingScreen setGameOverStatus";
}

TEST_F(UT_PlayingScreen, UT_PlayingScreen_boundingRect)
{
    m_PlayingScreen->boundingRect();
    EXPECT_EQ(m_PlayingScreen->sceneWidth, 300)
            << "check playingScreen boundingRect";
}

TEST_F(UT_PlayingScreen, UT_PlayingScreen_paint1)
{
    QStyleOptionGraphicsItem *option = nullptr;
    QWidget *widget = nullptr;
    QPainter painter;
    m_PlayingScreen->gamePlaying = false;
    m_PlayingScreen->paint(&painter, option, widget);
}

TEST_F(UT_PlayingScreen, UT_PlayingScreen_paint2)
{
    QStyleOptionGraphicsItem *option = nullptr;
    QWidget *widget = nullptr;
    QPainter painter;
    m_PlayingScreen->gamePlaying = true;
    m_PlayingScreen->gameOverStatus = true;
    m_PlayingScreen->AIPlayer = true;
    m_PlayingScreen->currentChessColro = chess_black;
    m_PlayingScreen->paint(&painter, option, widget);
}

TEST_F(UT_PlayingScreen, UT_PlayingScreen_paint3)
{
    QStyleOptionGraphicsItem *option = nullptr;
    QWidget *widget = nullptr;
    QPainter painter;
    m_PlayingScreen->gamePlaying = true;
    m_PlayingScreen->gameOverStatus = false;
    m_PlayingScreen->AIPlayer = false;
    m_PlayingScreen->currentChessColro = chess_white;
    m_PlayingScreen->paint(&painter, option, widget);
}

TEST_F(UT_PlayingScreen, UT_PlayingScreen_slotStartGame)
{
    m_PlayingScreen->slotStartGame();
    EXPECT_EQ(m_PlayingScreen->gamePlaying, true)
            << "check playingScreen slotStartGame";
}

TEST_F(UT_PlayingScreen, UT_PlayingScreen_slotNewGame)
{
    m_PlayingScreen->slotNewGame();
    EXPECT_EQ(m_PlayingScreen->gamePlaying, false)
            << "check playingScreen slotNewGame";
}
