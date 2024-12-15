// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_artificialintelligence.h"
#include "calculatescore.h"
#include "stub.h"

int getLocationScore_stub(const ChessState chessState,
                                     const int color, const Position position)
{
    if (position == std::make_pair(7, 7) && color == 1 && !chessState[7][7]) {
        return 100000;
    }
    return 0;
}

Position getBestLocation_stub(const int AIScore[line_row][line_col],
                                         const int opponentScore[line_row][line_col])
{
    Position position;
    for (int i = 0; i < line_row; i++) {
        for (int j = 0; j < line_col; j++) {
            if (AIScore[i][j] == 100000) {
                position = std::make_pair(i, j);
            }
        }
    }
    for (int i = 0; i < line_row; i++) {
        for (int j = 0; j < line_col; j++) {
            if (opponentScore[i][j] == 100000) {
                position = std::make_pair(i, j);
            }
        }
    }
    return position;
}

TEST_F(UT_Artificialintelligence, UT_Artificialintelligence)
{
    ArtificialIntelligence ai;
}

TEST_F(UT_Artificialintelligence, UT_Artificialintelligence_getPosition_001)
{
    ChessState chessState(line_row, QVector<int>(line_col, 0));
    EXPECT_EQ(ArtificialIntelligence::getPosition(chessState, 1), std::make_pair(6, 6))
            << "check Artificialintelligence getPosition_001";
}

TEST_F(UT_Artificialintelligence, UT_Artificialintelligence_getPosition_002)
{
    ChessState chessState = {
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,2,2,2,0,0,0,0,0,0},
           {0,0,0,0,0,1,1,0,0,0,0,0,0},
           {0,0,0,0,0,1,1,2,1,0,0,0,0},
           {0,0,0,0,2,1,1,1,2,0,0,0,0},
           {0,0,0,0,0,1,1,0,1,0,0,0,0},
           {0,0,0,0,0,2,2,1,2,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0}
       };
    Position position = std::make_pair(7, 7);
    Stub stub;
    stub.set(ADDR(CalculateScore, getLocationScore), getLocationScore_stub);
    stub.set(ADDR(CalculateScore, getBestLocation), getBestLocation_stub);
    EXPECT_EQ(ArtificialIntelligence::getPosition(chessState, 1), position)
            << "check Artificialintelligence getPosition_002";;
}
