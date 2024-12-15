// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_gamecontrol.h"

TEST_F(UT_GameControl, UT_GameControl_initGame1)
{
    m_GameControl->AIColor = chess_black;
    m_GameControl->initGame();
    EXPECT_EQ(m_GameControl->AIPlaying, true)
            << "check gameControl initGame";
}

TEST_F(UT_GameControl, UT_GameControl_initGame2)
{
    m_GameControl->AIColor = chess_white;
    m_GameControl->gameReset = true;
    m_GameControl->initGame();
    EXPECT_EQ(m_GameControl->AIPlaying, false)
            << "check gameControl initGame";
    EXPECT_EQ(m_GameControl->gameReset, false)
            << "check gameControl initGame";
}

TEST_F(UT_GameControl, UT_GameControl_startGame)
{
    m_GameControl->startGame();
    EXPECT_EQ(m_GameControl->initGameStatus, true)
            << "check gameControl startGame";
}

TEST_F(UT_GameControl, UT_GameControl_setChessColor)
{
    m_GameControl->setChessColor(chess_white, chess_black);
    EXPECT_EQ(m_GameControl->userColor, chess_black)
            << "check gameControl setChessColor";
}

TEST_F(UT_GameControl, UT_GameControl_chessCompleted)
{
    Chess chess(1, 1, chess_black);
    m_GameControl->chessCompleted(chess);
    EXPECT_EQ(m_GameControl->checkerboard->chessState[1][1], chess_black)
            << "check gameControl chessCompleted";
}

TEST_F(UT_GameControl, UT_GameControl_resetGame)
{
    m_GameControl->resetGame();
    EXPECT_EQ(m_GameControl->initGameStatus, true)
            << "check gameControl resetGame";
}

TEST_F(UT_GameControl, UT_GameControl_playChess)
{
    Chess chess(1, 1, chess_black);
    m_GameControl->playChess(chess);
}

TEST_F(UT_GameControl, UT_GameControl_setAIChess1)
{
    m_GameControl->gameReset = true;
    m_GameControl->setAIChess();
    EXPECT_EQ(m_GameControl->gameReset, false)
            << "check gameControl setAIChess";
}

TEST_F(UT_GameControl, UT_GameControl_setAIChess2)
{
    m_GameControl->gameReset = false;
    m_GameControl->AIPlaying = true;
    m_GameControl->initGameStatus = true;
    m_GameControl->setAIChess();
    EXPECT_EQ(m_GameControl->initGameStatus, false)
            << "check gameControl setAIChess";
}

TEST_F(UT_GameControl, UT_GameControl_setAIChess3)
{
    m_GameControl->gameReset = false;
    m_GameControl->AIPlaying = true;
    m_GameControl->initGameStatus = false;
    m_GameControl->setAIChess();
    EXPECT_EQ(m_GameControl->AIPlaying, false)
            << "check gameControl setAIChess";
}

