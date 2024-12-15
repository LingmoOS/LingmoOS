// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHESSFROMJUDGE_H
#define CHESSFROMJUDGE_H

#include "constants.h"
#include "pub.h"

#include <utility>

/**
 * @brief The ChessFrom enum 棋局形式枚举
 */
enum ChessFrom {
    lian5, //五连珠
    alive4, //活四
    sleep4,//眠四
    rush4, //冲四
    alive3, //活三
    rush3, //冲三
    sleep3, //眠三
    alive2, //活二
    rush2, //冲二
    sleep2, //眠二
    safety, //安全没有威胁
};

/**
 * @brief The Direction enum 方向枚举
 */
enum Direction {
    horizontal,//横向
    vertical, //竖向
    northeast,//右上左下
    northwest,//左上右下
};



class ChessFromJudge
{
public:
    ChessFromJudge();
    /**
     * @brief ChessTypeJudge
     * @param chess
     * @return
     * 棋子类型判断
     */
    static ChessFrom judgeChessFrom(const int chess[9]);

    /**
     * @brief judgeColor
     * @param color
     * @return
     * 判断另一个玩家的棋子颜色
     */
    static int judgeColor(const int color);

    /**
     * @brief collectChess 收集一个方向上的9个棋子
     * @param chess 存储收集好的棋子数组
     * @param chessState 棋盘状态
     * @param color 棋子颜色
     * @param position 位置
     * @param direction 方向
     */
    static void collectChess(int chess[9], const ChessState chessState,
                            const int color, const Position position, const int direction);
    /**
     * @brief judgeResult 棋局形式判断
     * @param chessState 棋盘状态数组
     * @param chess 最后一次落子数据
     * @return 返回胜利、平局、游戏进行中三种枚举值
     */
    static ChessResult judgeResult(const ChessState chessState, const Chess chess);
};

#endif // CHESSFROMJUDGE_H
