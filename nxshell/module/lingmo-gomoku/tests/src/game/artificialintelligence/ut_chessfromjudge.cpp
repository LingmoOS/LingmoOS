// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_chessfromjudge.h"
#include "constants.h"

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge)
{
    ChessFromJudge judge;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_001)
{
    int chess[9] = {0,0,2,2,2,2,2,0,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::lian5)
            << "check ChessFormJudge judgeChessForm_001" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_002)
{
    int chess[9] = {0,0,0,2,2,2,2,0,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::alive4)
            << "check ChessFormJudge judgeChessForm_002" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_003)
{
    int chess[9] = {0,0,1,2,2,2,2,1,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::safety)
            << "check ChessFormJudge judgeChessForm_003" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_004)
{
    int chess[9] = {0,0,0,2,2,2,2,1,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::sleep4)
            << "check ChessFormJudge judgeChessForm_004" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_005)
{
    int chess[9] = {0,0,1,0,2,2,2,0,1};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::sleep3)
            << "check ChessFormJudge judgeChessForm_005" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_006)
{
    int chess[9] = {0,0,1,0,2,2,2,0,2};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::rush4)
            << "check ChessFormJudge judgeChessForm_006" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_007)
{
    int chess[9] = {0,0,0,0,2,2,2,0,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::alive3)
            << "check ChessFormJudge judgeChessForm_007" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_008)
{
    int chess[9] = {0,0,0,1,2,2,2,1,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::safety)
            << "check ChessFormJudge judgeChessForm_008" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_009)
{
    int chess[9] = {0,0,0,1,2,2,2,0,1};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::safety)
            << "check ChessFormJudge judgeChessForm_009" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_010)
{
    int chess[9] = {0,0,0,1,2,2,2,0,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::sleep3)
            << "check ChessFormJudge judgeChessForm_010" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_011)
{
    int chess[9] = {0,0,0,1,2,2,2,0,2};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::rush4)
            << "check ChessFormJudge judgeChessForm_011" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_012)
{
    int chess[9] = {0,0,1,0,2,2,2,1,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::safety)
            << "check ChessFormJudge judgeChessForm_012" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_013)
{
    int chess[9] = {0,0,0,0,2,2,2,1,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::sleep3)
            << "check ChessFormJudge judgeChessForm_013" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_014)
{
    int chess[9] = {0,0,2,0,2,2,2,1,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::rush4)
            << "check ChessFormJudge judgeChessForm_014" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_015)
{
    int chess[9] = {0,0,0,2,2,0,0,2,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::sleep3)
            << "check ChessFormJudge judgeChessForm_015" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_016)
{
    int chess[9] = {0,0,0,0,2,2,0,0,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::alive2)
            << "check ChessFormJudge judgeChessForm_016" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_017)
{
    int chess[9] = {0,0,1,0,2,2,0,1,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::safety)
            << "check ChessFormJudge judgeChessForm_017" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_018)
{
    int chess[9] = {0,0,1,0,2,2,0,0,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::sleep2)
            << "check ChessFormJudge judgeChessForm_018" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_019)
{
    int chess[9] = {0,1,2,0,2,2,0,0,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::sleep3)
            << "check ChessFormJudge judgeChessForm_019" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_020)
{
    int chess[9] = {0,2,2,0,2,2,0,0,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::rush4)
            << "check ChessFormJudge judgeChessForm_020" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_021)
{
    int chess[9] = {0,0,2,0,2,2,0,0,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::rush3)
            << "check ChessFormJudge judgeChessForm_021" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_022)
{
    int chess[9] = {0,0,0,1,2,2,1,1,1};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::safety)
            << "check ChessFormJudge judgeChessForm_022" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_023)
{
    int chess[9] = {0,0,0,1,2,2,0,1,1};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::safety)
            << "check ChessFormJudge judgeChessForm_023" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_024)
{
    int chess[9] = {0,0,0,1,2,2,0,0,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::sleep2)
            << "check ChessFormJudge judgeChessForm_024" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_025)
{
    int chess[9] = {0,0,0,1,2,2,0,2,2};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::rush4)
            << "check ChessFormJudge judgeChessForm_025" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_026)
{
    int chess[9] = {0,0,0,1,2,2,0,2,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::sleep3)
            << "check ChessFormJudge judgeChessForm_026" ;
}
TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_027)
{
    int chess[9] = {0,1,1,0,2,2,1,0,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::safety)
            << "check ChessFormJudge judgeChessForm_027" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_028)
{
    int chess[9] = {0,0,0,0,2,2,1,0,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::sleep2)
            << "check ChessFormJudge judgeChessForm_028" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_029)
{
    int chess[9] = {0,2,2,0,2,2,1,0,1};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::rush4)
            << "check ChessFormJudge judgeChessForm_029" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_030)
{
    int chess[9] = {0,0,2,0,2,2,1,0,1};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::sleep3)
            << "check ChessFormJudge judgeChessForm_030" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_031)
{
    int chess[9] = {0,0,0,0,2,0,2,2,2};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::rush4)
            << "check ChessFormJudge judgeChessForm_031" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_032)
{
    int chess[9] = {2,2,2,0,2,0,0,0,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::rush4)
            << "check ChessFormJudge judgeChessForm_032" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_033)
{
    int chess[9] = {0,2,2,0,2,0,0,0,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::rush3)
            << "check ChessFormJudge judgeChessForm_033" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_034)
{
    int chess[9] = {0,0,0,0,2,0,2,2,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::rush3)
            << "check ChessFormJudge judgeChessForm_034" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_035)
{
    int chess[9] = {1,2,2,0,2,0,0,0,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::sleep3)
            << "check ChessFormJudge judgeChessForm_035" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_036)
{
    int chess[9] = {0,0,0,0,2,0,2,2,1};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::sleep3)
            << "check ChessFormJudge judgeChessForm_036" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_037)
{
    int chess[9] = {2,2,0,0,2,0,0,0,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::sleep3)
            << "check ChessFormJudge judgeChessForm_037" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_038)
{
    int chess[9] = {0,0,0,0,2,0,0,2,2};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::sleep3)
            << "check ChessFormJudge judgeChessForm_038" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_039)
{
    int chess[9] = {0,0,0,0,2,0,2,0,2};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::sleep3)
            << "check ChessFormJudge judgeChessForm_039" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_040)
{
    int chess[9] = {2,0,2,0,2,0,0,0,};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::sleep3)
            << "check ChessFormJudge judgeChessForm_040" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_041)
{
    int chess[9] = {0,0,2,0,2,0,0,0,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::rush2)
            << "check ChessFormJudge judgeChessForm_041" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_042)
{
    int chess[9] = {0,0,0,0,2,0,2,0,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::rush2)
            << "check ChessFormJudge judgeChessForm_042" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_043)
{
    int chess[9] = {0,0,0,0,2,0,0,2,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::rush2)
            << "check ChessFormJudge judgeChessForm_043" ;
}
TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeChessFrom_044)
{
    int chess[9] = {0,2,0,0,2,0,0,0,0};
    EXPECT_EQ(ChessFromJudge::judgeChessFrom(chess), ChessFrom::rush2)
            << "check ChessFormJudge judgeChessForm_044" ;
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeColor_001)
{
    EXPECT_EQ(ChessFromJudge::judgeColor(1), 2) << "check ChessFromJudge judgeColor 001";
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeColor_002)
{
    EXPECT_EQ(ChessFromJudge::judgeColor(2), 1) << "check ChessFromJudge judgeColor 002";
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeResult_001)
{
    ChessState chessState = {
           {1,1,1,1,1,0,0,0,2,2,2,2,2},
           {0,0,0,1,0,0,0,0,0,0,0,0,0},
           {0,0,1,0,0,0,0,0,0,0,0,0,0},
           {0,1,0,0,0,0,0,0,0,0,0,0,0},
           {1,0,0,0,0,1,0,0,0,0,0,0,0},
           {0,0,0,0,0,1,0,0,0,0,0,0,0},
           {0,0,0,0,0,1,0,0,0,0,0,0,0},
           {0,0,0,0,0,1,0,0,0,0,0,0,0},
           {0,0,0,0,0,1,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,1,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,1,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,1,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,1,0,0,0}
       };

    EXPECT_EQ(ChessFromJudge::judgeResult(chessState, Chess(-1, -1, 1)), ChessResult::playing)
            << "check ChessFromJudge judgeResult << 游戏中"; // 游戏中

    EXPECT_EQ(ChessFromJudge::judgeResult(chessState, Chess(5, 5, 1)), ChessResult::black_win)
            << "check ChessFromJudge judgeResult << 竖向"; //竖向

    EXPECT_EQ(ChessFromJudge::judgeResult(chessState, Chess(0, 0, 1)), ChessResult::black_win)
            << "check ChessFromJudge judgeResult << 横向"; //横向

    EXPECT_EQ(ChessFromJudge::judgeResult(chessState, Chess(4, 0, 1)), ChessResult::black_win)
            << "check ChessFromJudge judgeResult << 右上"; //右上

    EXPECT_EQ(ChessFromJudge::judgeResult(chessState, Chess(12, 9, 1)), ChessResult::black_win)
            << "check ChessFromJudge judgeResult << 左下"; //左下

    EXPECT_EQ(ChessFromJudge::judgeResult(chessState, Chess(0, 8, 2)), ChessResult::white_win)
            << "check ChessFromJudge judgeResult << 白子赢"; //白子赢
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_judgeResult_002)
{

    ChessState chessState (line_row, QVector<int>(line_col, 1));
    EXPECT_EQ(ChessFromJudge::judgeResult(chessState,Chess(12, 0, 1)), ChessResult::tie)
            << "check ChessFromJudge judgeResult 002"; //平局
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_collectChess_001)
{
    ChessState chessState = {
           {0,0,0,0,0,0,0,0,2,1,1,1,2},
           {0,0,0,1,0,0,0,0,0,0,0,0,0},
           {0,0,1,0,0,0,0,0,0,0,0,0,0},
           {0,1,0,0,0,0,0,0,0,0,0,0,0},
           {1,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0}
       };
    int chess[9] = {0};
    int chessTest[9] = {0,0,2,1,1,1,2,2,2};
    ChessFromJudge::collectChess(chess, chessState, 1, std::make_pair(0,10), Direction::horizontal);
    std::vector <int> vChess(chess, chess + 9);
    std::vector <int> vChessTest(chessTest, chessTest + 9);
    EXPECT_EQ(vChess, vChessTest) << "check ChessFromJudge collectChess 001";
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_collectChess_002)
{
    ChessState chessState = {
           {0,0,0,1,0,0,0,0,2,1,1,1,2},
           {0,0,0,1,0,0,0,0,0,0,0,0,0},
           {0,0,1,2,0,0,0,0,0,0,0,0,0},
           {0,1,0,2,0,0,0,0,0,0,0,0,0},
           {1,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0}
       };
    int chess[9] = {0};
    int chessTest[9] = {2,2,2,2,1,1,2,2,0};
    ChessFromJudge::collectChess(chess, chessState, 1, std::make_pair(0,3), Direction::vertical);
    std::vector <int> vChess(chess, chess + 9);
    std::vector <int> vChessTest(chessTest, chessTest + 9);
    EXPECT_EQ(vChess, vChessTest) << "check ChessFromJudge collectChess 002";
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_collectChess_003)
{
    ChessState chessState = {
           {0,0,0,0,0,0,0,0,2,1,1,1,2},
           {0,0,0,1,0,0,0,0,0,0,0,0,0},
           {0,0,1,0,0,0,0,0,0,0,0,0,0},
           {0,1,0,0,0,0,0,0,0,0,0,0,0},
           {1,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,1,0,0,0,0,0,0,0},
           {0,0,0,0,1,0,0,0,0,0,0,0,0},
           {0,0,0,1,0,0,0,0,0,0,0,0,0},
           {0,0,2,0,0,0,0,0,0,0,0,0,0},
           {0,1,0,0,0,0,0,0,0,0,0,0,0},
           {1,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0}
       };
    int chess[9] = {0};
    int chessTest[9] = {2,1,1,2,1,1,1,0,0};
    ChessFromJudge::collectChess(chess, chessState, 1, std::make_pair(7, 3), Direction::northeast);
    std::vector <int> vChess(chess, chess + 9);
    std::vector <int> vChessTest(chessTest, chessTest + 9);
    EXPECT_EQ(vChess, vChessTest) << "check ChessFromJudge collectChess 003";
}

TEST_F(UT_ChessFromJudge, UT_ChessFromJudge_collectChess_004)
{
    ChessState chessState = {
           {0,0,0,0,0,0,0,0,2,1,1,1,2},
           {0,0,0,1,0,0,0,0,0,0,0,0,0},
           {0,0,1,0,0,0,0,0,0,0,0,0,0},
           {0,1,0,0,0,0,0,0,0,0,0,0,0},
           {1,0,0,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,1,0,0,0,0,0,0,0},
           {0,0,0,0,1,0,0,0,0,0,0,0,0},
           {0,0,0,1,0,0,0,0,0,0,0,0,0},
           {0,0,2,0,0,0,0,0,0,0,0,0,0},
           {0,1,0,0,0,0,0,0,0,0,0,0,0},
           {1,0,1,0,0,0,0,0,0,0,0,0,0},
           {0,0,0,1,0,0,0,0,0,0,0,0,0},
           {0,0,0,0,0,0,0,0,0,0,0,0,0}
       };
    int chess[9] = {0};
    int chessTest[9] = {2,2,0,1,1,1,0,2,2};
    ChessFromJudge::collectChess(chess, chessState, 1, std::make_pair(10, 2), Direction::northwest);
    std::vector <int> vChess(chess, chess + 9);
    std::vector <int> vChessTest(chessTest, chessTest + 9);
    EXPECT_EQ(vChess, vChessTest) << "check ChessFromJudge collectChess 004";
}
