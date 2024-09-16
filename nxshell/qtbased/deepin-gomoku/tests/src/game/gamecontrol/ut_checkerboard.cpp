// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_checkerboard.h"

TEST_F(UT_CheckerBoard, UT_CheckerBoard_insertChess)
{
    Chess chess(1, 1, chess_black);
    m_CheckerBoard->insertChess(chess);
    EXPECT_EQ(m_CheckerBoard->chessState[1][1], chess_black)
            << "check checkerBoard insertChess";
}

TEST_F(UT_CheckerBoard, UT_CheckerBoard_getChessState)
{
    Chess chess(1, 1, chess_black);
    m_CheckerBoard->insertChess(chess);
    ChessState res = m_CheckerBoard->getChessState();
    EXPECT_EQ(res[1][1], chess_black)
            << "check checkerBoard getChessState";
}

TEST_F(UT_CheckerBoard, UT_CheckerBoard_initCheckerboard)
{
    m_CheckerBoard->initCheckerboard();
    EXPECT_EQ(m_CheckerBoard->chessState[1][1], chess_none)
            << "check checkerBoard initCheckerboard";
}

TEST_F(UT_CheckerBoard, UT_CheckerBoard_resetCheckerboard)
{
    m_CheckerBoard->resetCheckerboard();
    EXPECT_EQ(m_CheckerBoard->chessState[1][1], chess_none)
            << "check checkerBoard initCheckerboard";
}

