// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_chessitem.h"
#include "constants.h"

#include <QPainter>
#include <QGraphicsSceneHoverEvent>

TEST_F(UT_ChessItem, UT_ChessItem_setCurrentchess)
{
    m_ChessItem->setCurrentchess(1);
    EXPECT_EQ(m_ChessItem->chessType, 1)
            << "check chessItem setcurrentchess";
}

TEST_F(UT_ChessItem, UT_ChessItem_getChessColor)
{
    m_ChessItem->setCurrentchess(1);
    int res = m_ChessItem->getChessColor();
    EXPECT_EQ(res, 1)
            << "check chessItem getchessColor";
}

TEST_F(UT_ChessItem, UT_ChessItem_setHasPrintChess)
{
    m_ChessItem->setHasPrintChess();
    EXPECT_EQ(m_ChessItem->chessHasPrint, true)
            << "check chessItem setHasPrintChess";
}

TEST_F(UT_ChessItem, UT_ChessItem_setchessStatus)
{
    m_ChessItem->setchessStatus(true);
    EXPECT_EQ(m_ChessItem->chessStatus, true)
            << "check chessItem setchessStatus";
}

TEST_F(UT_ChessItem, UT_ChessItem_getchessStatus)
{
    m_ChessItem->setchessStatus(true);
    bool res = m_ChessItem->getchessStatus();
    EXPECT_EQ(res, true)
            << "check chessItem getchessStatus";
}

TEST_F(UT_ChessItem, UT_ChessItem_getChessPlayer)
{
    m_ChessItem->slotIsAIPlaying(false);
    int res = m_ChessItem->getChessPlayer();
    EXPECT_EQ(res, 0)
            << "check chessItem getChessPlayer";
}

TEST_F(UT_ChessItem, UT_ChessItem_hoverHandle)
{
    m_ChessItem->gameOver = false;
    m_ChessItem->gameStatus = true;
    m_ChessItem->chessStatus = false;
    m_ChessItem->slotIsAIPlaying(false);
    m_ChessItem->m_isCurrentItem = true;
    m_ChessItem->hoverHandle();
    EXPECT_EQ(m_ChessItem->hoverStatus, true)
            << "check chessItem hoverHandle";
}

TEST_F(UT_ChessItem, UT_ChessItem_paint1)
{
    QStyleOptionGraphicsItem *option = nullptr;
    QWidget *widget = nullptr;
    QPainter painter;
    m_ChessItem->chessStatus = true;
    m_ChessItem->chessType = chess_white;
    m_ChessItem->chessHasPrint = true;
    m_ChessItem->paint(&painter, option, widget);
}

TEST_F(UT_ChessItem, UT_ChessItem_paint2)
{
    QStyleOptionGraphicsItem *option = nullptr;
    QWidget *widget = nullptr;
    QPainter painter;
    m_ChessItem->chessStatus = true;
    m_ChessItem->chessType = chess_black;
    m_ChessItem->chessHasPrint = false;
    m_ChessItem->paint(&painter, option, widget);
}

TEST_F(UT_ChessItem, UT_ChessItem_paint3)
{
    QStyleOptionGraphicsItem *option = nullptr;
    QWidget *widget = nullptr;
    QPainter painter;
    m_ChessItem->chessStatus = false;
    m_ChessItem->hoverStatus = true;
    m_ChessItem->chessType = chess_black;
    m_ChessItem->paint(&painter, option, widget);
}

TEST_F(UT_ChessItem, UT_ChessItem_paint4)
{
    QStyleOptionGraphicsItem *option = nullptr;
    QWidget *widget = nullptr;
    QPainter painter;
    m_ChessItem->chessStatus = false;
    m_ChessItem->hoverStatus = true;
    m_ChessItem->chessType = chess_white;
    m_ChessItem->paint(&painter, option, widget);
}

TEST_F(UT_ChessItem, UT_ChessItem_boundingRect)
{
    m_ChessItem->boundingRect();
    EXPECT_EQ(m_ChessItem->boundingRect().width(), chess_size)
            << "check chessItem boundingRect";
}

TEST_F(UT_ChessItem, UT_ChessItem_hoverEnterEvent)
{
    QGraphicsSceneHoverEvent event;
    m_ChessItem->hoverEnterEvent(&event);
    EXPECT_EQ(m_ChessItem->m_isCurrentItem, true)
            << "check chessItem hoverEnterEvent";
}

TEST_F(UT_ChessItem, UT_ChessItem_hoverLeaveEvent)
{
    QGraphicsSceneHoverEvent event;
    m_ChessItem->hoverLeaveEvent(&event);
    EXPECT_EQ(m_ChessItem->m_isCurrentItem, false)
            << "check chessItem hoverLeaveEvent";
}

TEST_F(UT_ChessItem, UT_ChessItem_mousePressEvent)
{
    QGraphicsSceneMouseEvent event;
    event.setButton(Qt::MouseButton::LeftButton);
    m_ChessItem->mousePressEvent(&event);
}

TEST_F(UT_ChessItem, UT_ChessItem_mouseReleaseEvent)
{
    QGraphicsSceneMouseEvent event;
    event.setButton(Qt::MouseButton::LeftButton);
    m_ChessItem->mouseReleaseEvent(&event);
}

TEST_F(UT_ChessItem, UT_ChessItem_slotGameOver)
{
    m_ChessItem->slotGameOver();
    EXPECT_EQ(m_ChessItem->gameOver, true)
            << "check chessItem slotGameOver";
}

TEST_F(UT_ChessItem, UT_ChessItem_slotIsAIPlaying)
{
    m_ChessItem->slotIsAIPlaying(true);
    EXPECT_EQ(m_ChessItem->isAIPlaying, true)
            << "check chessItem slotIsAIPlaying";
}

TEST_F(UT_ChessItem, UT_ChessItem_slotGameContinue)
{
    m_ChessItem->slotGameContinue();
    EXPECT_EQ(m_ChessItem->gameStatus, true)
            << "check chessItem slotGameContinue";
}

TEST_F(UT_ChessItem, UT_ChessItem_slotGameStop)
{
    m_ChessItem->slotGameStop();
    EXPECT_EQ(m_ChessItem->gameStatus, false)
            << "check chessItem slotGameStop";
}
