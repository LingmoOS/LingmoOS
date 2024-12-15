// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_checkerboardscene.h"

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_initChess)
{
    m_checkerboardScene->initChess();
    EXPECT_EQ(m_checkerboardScene->chessItemList.isEmpty(), false)
            << "check checkerboardScene initChess";
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_removeButtonFunction)
{
    m_checkerboardScene->removeButtonFunction();
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_addButtonFunction)
{
    m_checkerboardScene->removeButtonFunction();
    m_checkerboardScene->addButtonFunction();
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_setAIChess)
{
    Chess chess(1, 1, chess_black);
    m_checkerboardScene->setAIChess(chess);
    EXPECT_EQ(m_checkerboardScene->chessItemList.at(chess.x).at(chess.y)->getchessStatus(), true)
            << "check CheckerboardScene setAIChess";
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_slotGameStart)
{
    m_checkerboardScene->slotGameStart();
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_replayFunction)
{
    m_checkerboardScene->replayFunction();
    EXPECT_EQ(m_checkerboardScene->gameReplay, true)
            << "check checkerboardScene replayFunction";
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_setStartPauseStatus)
{
    m_checkerboardScene->setStartPauseStatus();
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_initGame)
{
    m_checkerboardScene->initGame();
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_slotGameStop)
{
    m_checkerboardScene->slotGameStop();
    EXPECT_EQ(m_checkerboardScene->gameStatus, false)
            << "check checkerboardScene slotGameStop";
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_slotGameContinue)
{
    m_checkerboardScene->slotGameContinue();
    EXPECT_EQ(m_checkerboardScene->gameStatus, true)
            << "check checkerboardScene slotGameContinue";
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_slotGameOver)
{
    m_checkerboardScene->slotGameOver(ChessResult::tie);
    EXPECT_EQ(m_checkerboardScene->m_gameState, GameState::gameOver)
            << "check checkerboardScene slotGameOver";
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_selsectChessOK)
{
    m_checkerboardScene->selsectChessOK();
    EXPECT_EQ(m_checkerboardScene->gameStatus, true)
            << "check checkerboardScene selsectChessOK";
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_getMusicPlay)
{
    m_checkerboardScene->musicControlStatus = true;
    bool res = m_checkerboardScene->getMusicPlay();
    EXPECT_EQ(res, true) << "check checkerboardScene getMusicPlay";
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_getUserChessColor)
{
    m_checkerboardScene->setSelectChess(chess_black, chess_white);
    int chessColor = m_checkerboardScene->getUserChessColor();
    EXPECT_EQ(chessColor, chess_black)
            << "check checkerboardScene getUserChessColor";
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_slotCPaintItem_black)
{
    m_checkerboardScene->musicControlStatus = true;
    m_checkerboardScene->chessItemList.at(1).at(1)->setCurrentchess(chess_black);
    m_checkerboardScene->slotCPaintItem(m_checkerboardScene->chessItemList.at(1).at(1));
    EXPECT_EQ(m_checkerboardScene->clickPosCol, 1)
            << "check checkerboardScene slotCPaintItem black";
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_slotCPaintItem_white)
{
    m_checkerboardScene->musicControlStatus = true;
    m_checkerboardScene->chessItemList.at(1).at(1)->setCurrentchess(chess_white);
    m_checkerboardScene->slotCPaintItem(m_checkerboardScene->chessItemList.at(1).at(1));
    EXPECT_EQ(m_checkerboardScene->clickPosRow, 1)
            << "check checkerboardScene slotCPaintItem white";
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_startGame)
{
    m_checkerboardScene->startGame();
    EXPECT_EQ(m_checkerboardScene->gameReplay, false)
            << "check checkerboardScene startGame";
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_stopGame)
{
    m_checkerboardScene->stopGAme();
    EXPECT_EQ(m_checkerboardScene->buttonStartPause->mouseReleased, true)
            << "check checkerboardScene stopGame";
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_setSelectChess)
{
    m_checkerboardScene->setSelectChess(chess_black, chess_white);
    EXPECT_EQ(m_checkerboardScene->userChessColor, chess_black)
            << "check checkerboardScene setSelectChess";
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_slotPaintAIChess)
{
    Chess chess(1, 1, chess_black);
    m_checkerboardScene->gameReplay = false;
    m_checkerboardScene->gameStatus = true;
    m_checkerboardScene->slotPaintAIChess(chess);
    EXPECT_EQ(m_checkerboardScene->chessItemList.at(chess.x).at(chess.y)->getChessColor(), chess_black)
            << "check checkerboardScene slotPaintAIChess";
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_slotPaintAIChess1)
{
    Chess chess(1, 1, chess_black);
    m_checkerboardScene->gameStatus = false;
    m_checkerboardScene->slotPaintAIChess(chess);
    EXPECT_EQ(m_checkerboardScene->AIChessStatus, true)
            << "check checkerboardScene slotPaintAIChess";
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_drawBackground)
{
    QPainter painter;
    QRectF rect;
    m_checkerboardScene->drawBackground(&painter, rect);
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_setGameState)
{
    m_checkerboardScene->setGameState(GameState::gamePlaying);
    EXPECT_EQ(m_checkerboardScene->getGameState(), GameState::gamePlaying)
            << "check checkerboardScene setGameState";
}

TEST_F(UT_CheckerboardScene, UT_CheckerboardScene_getGameState)
{
    m_checkerboardScene->setGameState(GameState::gamePlaying);
    GameState res = m_checkerboardScene->getGameState();
    EXPECT_EQ(res, GameState::gamePlaying)
            << "check checkerboardScene getGameState";
}
