// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_calculatescore.h"
#include "stub.h"
#include <unordered_map>

ChessFrom judgeChessFrom_stub()
{
    return safety;
}

void collectChess_stub()
{
    return;
}


TEST_F(UT_CalculateScore, UT_CalculateScore)
{
    CalculateScore calculateScore;
}

TEST_F(UT_CalculateScore, UT_CalculateScore_getLocationScore_001)
{
    ChessState chessState = {
           {1,1,1,0,0,0,0,0,0,1,1,1,1},
           {0,0,0,1,0,0,0,0,0,0,0,0,0},
           {0,0,0,1,0,0,0,0,0,0,0,0,0},
           {0,0,0,1,0,0,0,0,0,0,0,0,0},
           {0,1,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,1,1,0,0,0,0,0,0,0,0,0},
           {0,1,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,1,0,0,0,0,0,0,0,0,0,0},
           {1,1,0,0,0,0,0,0,0,0,0,1,0},
           {0,0,1,0,0,0,0,0,0,0,0,0,1},
           {0,0,0,0,0,0,0,0,0,0,1,1,1}
       };

    EXPECT_EQ(CalculateScore::getLocationScore(chessState, 1, std::make_pair(0, 0)), 0)
            << "check CalculateScore getLocationScore_001";
}

TEST_F(UT_CalculateScore, UT_CalculateScore_getLocationScore_002)
{
    ChessState chessState = {
           {1,1,1,0,0,0,0,0,0,1,1,1,1},
           {0,0,0,1,0,0,0,0,0,0,0,0,0},
           {0,0,0,1,0,0,0,0,0,0,0,0,0},
           {0,0,0,1,0,0,0,0,0,0,0,0,0},
           {0,1,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,1,1,0,0,0,0,0,0,0,0,0},
           {0,1,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,1,0,0,0,0,0,0,0,0,0,0},
           {1,1,0,0,0,0,0,0,0,0,0,1,0},
           {0,0,1,0,0,0,0,0,0,0,0,0,1},
           {0,0,0,0,0,0,0,0,0,0,1,1,1}
       };
    Stub stub;
    stub.set(ADDR(ChessFromJudge, collectChess), collectChess_stub);

    stub.set(ADDR(ChessFromJudge, judgeChessFrom), judgeChessFrom_stub);
    EXPECT_EQ(CalculateScore::getLocationScore(chessState, 1, std::make_pair(0, 4)), 1)
            << "check CalculateScore getLocationScore_002_";

}

TEST_F(UT_CalculateScore, UT_CalculateScore_computeScore)
{
    std::unordered_map<ChessFrom, int> m;
    m[ChessFrom::safety] = 4;
    EXPECT_EQ(CalculateScore::computeScore(m), 1) <<
            "check CalculateScore computeScore";
}

TEST_F(UT_CalculateScore, UT_CalculateScore_getBestLocation_001)
{
    int ai[line_row][line_col] = {
           {1,1,1,5,0,0,0,0,0,1,1,1,1},
           {0,0,0,1,0,0,0,0,0,0,0,0,0},
           {0,2,0,1,0,0,0,0,0,0,0,0,0},
           {0,0,0,1,0,0,0,0,0,0,0,0,0},
           {0,1,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,1,1,0,0,0,0,0,0,0,0,0},
           {0,1,0,0,0,0,0,0,0,0,0,0,0},
           {0,2,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,1,0,2,0,0,0,0,0,0,0,0},
           {1,1,0,0,0,0,0,0,0,0,0,1,0},
           {0,2,1,0,2,0,0,0,0,0,0,0,1},
           {0,0,0,0,0,0,0,0,0,0,1,1,1}
       };
    int user[line_row][line_col] = {
           {1,1,1,0,0,0,0,0,0,1,1,1,1},
           {0,0,0,1,0,0,0,0,0,0,0,0,0},
           {0,2,0,1,0,0,0,0,0,0,2,0,0},
           {0,0,0,1,0,0,0,0,0,0,0,0,0},
           {0,1,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,1,1,0,0,0,0,2,0,2,0,0},
           {0,1,0,0,0,0,0,0,2,0,0,0,0},
           {0,2,0,0,0,0,0,0,2,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,1,0,2,0,0,0,0,0,0,0,0},
           {1,1,0,0,0,0,0,0,0,0,0,1,0},
           {0,2,1,0,2,0,0,0,0,0,0,0,1},
           {0,0,0,0,0,0,0,0,0,0,1,1,1}
       };
    EXPECT_EQ(CalculateScore::getBestLocation(ai, user), std::make_pair(0, 3))
            << "check CalculateScore getBestLocation_001";
}

TEST_F(UT_CalculateScore, UT_CalculateScore_getBestLocation_002)
{
    int ai[line_row][line_col] = {
           {1,1,1,1,0,0,0,0,0,1,1,1,1},
           {0,0,0,1,0,0,0,0,0,0,0,0,0},
           {0,2,0,1,0,0,0,0,0,0,0,0,0},
           {0,0,0,1,0,0,0,0,0,0,0,0,0},
           {0,1,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,1,1,0,0,0,0,0,0,0,0,0},
           {0,1,0,0,0,0,0,0,0,0,0,0,0},
           {0,2,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,1,0,2,0,0,0,0,0,0,0,0},
           {1,1,0,0,0,0,0,0,0,0,0,1,0},
           {0,2,1,0,2,0,0,0,0,0,0,0,1},
           {0,0,0,0,0,0,0,0,0,0,1,1,1}
       };
    int user[line_row][line_col] = {
           {1,1,1,5,0,0,0,0,0,1,1,1,1},
           {0,0,0,1,0,0,0,0,0,0,0,0,0},
           {0,2,0,1,0,0,0,0,0,0,2,0,0},
           {0,0,0,1,0,0,0,0,0,0,0,0,0},
           {0,1,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,1,1,0,0,0,0,2,0,2,0,0},
           {0,1,0,0,0,0,0,0,2,0,0,0,0},
           {0,2,0,0,0,0,0,0,2,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,1,0,2,0,0,0,0,0,0,0,0},
           {1,1,0,0,0,0,0,0,0,0,0,1,0},
           {0,2,1,0,2,0,0,0,0,0,0,0,1},
           {0,0,0,0,0,0,0,0,0,0,1,1,1}
       };
    EXPECT_EQ(CalculateScore::getBestLocation(ai, user), std::make_pair(0, 3))
            << "check CalculateScore getBestLocation_002";
}
