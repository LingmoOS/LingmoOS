// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "chessfromjudge.h"

ChessFromJudge::ChessFromJudge()
{

}

/**
 * @brief ChessFromJudge::judgeChessFrom 根据收集的9个棋子，判断该方向棋局形势
 * @param chess 收集的9个棋子
 * @return
 */
ChessFrom ChessFromJudge::judgeChessFrom(const int chess[9])
{
    int myColor = chess[4]; //自己棋子颜色
    int opponentColor; //对手的棋子颜色
    int count = 1; //统计相同棋子
    int left = 0, right = 0; //记录从中间往左数第几个不是同色的位置
    int leftColor = 0, rightColor = 0; //记录断开时该位置的棋子颜色

    opponentColor = ChessFromJudge::judgeColor(myColor); //记录对手棋子颜色

    //计算左边4个棋子
    for (int i = 1; i <= 4; i++) {
        if (chess[4 - i] == myColor) {
            count++; //从中心往左数有几个连子
        } else {
            left = 4 - i; //棋子颜色不同时记录位置跟颜色
            leftColor = chess[4 - i];
            break;
        }
    }

    //统计右边四个棋子
    for (int i = 1; i <= 4; i++) {
        if (chess[4 + i] == myColor) {
            count++;
        } else {
            right  = 4 + i;
            rightColor = chess[4 + i];
            break;
        }
    }

    //五连珠
    if (count >= 5) {
        return lian5;
    }

    //四连珠
    //已经达成四连珠，需判断是眠四、活四还是两面都堵死了没有威胁
    if (count == 4) {
        if (leftColor == chess_none && rightColor == chess_none) {
            return alive4;//两边都是空的，活四
        } else if (rightColor == opponentColor && leftColor == opponentColor) {
            return safety; //两边都被堵死了，没有威胁
        } else if (rightColor == chess_none || leftColor == chess_none) {
            return sleep4; //有一边为空，眠四
        }
    }

    //三连珠
    if (count == 3) {
        //记录下断开位置旁边的位置的棋子颜色，用来判断这个三连能否有五连的机会
        int leftColor1  = chess[left - 1];
        int rightColor1 = chess[right + 1];

        //三连珠两边位置都空，有五连珠的机会
        if (leftColor == chess_none && rightColor == chess_none) {
            if (leftColor1 == opponentColor && rightColor1 == opponentColor) { //被对手封住
                return sleep3;//1022201形式 眠三
            } else if (leftColor1 == myColor || rightColor1 == myColor) { //有一方是自己的棋子
                return rush4; //1022202形式 冲四
            } else if (leftColor1 == chess_none && rightColor1 == chess_none) { //两边都为空
                return alive3; //0022200形式 活三
            }

        }

        //三连两边都被封住，没有威胁
        if (leftColor == opponentColor && rightColor == opponentColor) {
            return safety;//12221形式 安全没威胁
        }

        //三连珠两边只有一个是空的
        if (leftColor == chess_none || rightColor == chess_none) {
            //分两种情况，左边被封和右边被封
            //左边被封
            if (leftColor == opponentColor) {
                if (rightColor1 == opponentColor) { //右边被堵了，无法五连
                    return safety; //122201形式 安全无威胁
                }
                if (rightColor1 == chess_none) { //右边是开的
                    return sleep3; //122200形式 眠三
                }
                if (rightColor1 == myColor) { //右边是自己的棋子
                    return rush4; //122202形式 冲四
                }
            }

            //右边被封 和左边被封同理
            if (rightColor == opponentColor) {
                if (leftColor1 == opponentColor) {
                    return safety;
                }
                if (leftColor1 == chess_none) {
                    return sleep3;
                }
                if (leftColor1 == myColor) {
                    return rush4;
                }
            }

        }

    }

    //二连
    if (count == 2) {
        //记录二连两边各三个位置
        int leftColor1 = chess[left - 1];
        int leftColor2 = chess[left - 2];
        int rightColor1 = chess[right + 1];
        int rightColor2 = chess[right + 2];

        //二连两边都为空
        if (leftColor == chess_none && rightColor == chess_none) {
            if ((rightColor1 == chess_none && rightColor2 == myColor) ||  //022002或020022
                    (leftColor1 == chess_none && leftColor2 == myColor)) {
                return sleep3;//眠三
            } else if (leftColor1 == chess_none && rightColor1 == chess_none) {
                return alive2;// 002200形式 活二
            } else if (leftColor1 == opponentColor && rightColor1 == opponentColor) {
                return  safety; // 102201
            } else if (leftColor1 == opponentColor || rightColor1 == opponentColor) {
                return sleep2; // 102200 002201
            }

            if ((rightColor1 == myColor && rightColor2 == opponentColor) ||
                    (leftColor1 == myColor && leftColor2 == opponentColor)) {
                return sleep3; // 120220 或 022021 形式 眠三
            }
            //022022 或 220220
            if ((rightColor1 == myColor && rightColor2 == myColor) ||
                    (leftColor1 == myColor && leftColor2 == myColor)) {
                return rush4; //冲四
            }
            // 020220 //冲三
            if ((rightColor1 == myColor && rightColor2 == chess_none) ||
                    (leftColor1 == myColor && leftColor2 == chess_none)) {
                return rush3; //冲三
            }
        } else if (leftColor == opponentColor && rightColor == opponentColor) { //两边都被堵死
            return safety; //安全没威胁
        } else if (leftColor == chess_none || rightColor == chess_none) { //一方被堵死
            if (leftColor == opponentColor) { //左边被堵死 1220
                if (rightColor1 == opponentColor || rightColor2 == opponentColor) { //右边也被堵死 122011
                    return safety; //安全没威胁
                } else if (rightColor1 == chess_none && rightColor2 == chess_none) {  //122000
                    return sleep2; //眠二
                } else if (rightColor1 == myColor && rightColor2 == myColor) { //右边是自己棋子 122022
                    return rush4; //冲四
                } else if (rightColor1 == myColor || rightColor2 == myColor) { //右边是自己的棋子 122020 或 122002
                    return sleep3; //眠三
                }
            }
            if (rightColor == opponentColor) { //右边被堵死 0221
                if (leftColor1 == opponentColor || leftColor2 == opponentColor) { //左边也被堵死 110221
                    return safety; //安全没威胁
                } else if (leftColor1 == chess_none && leftColor2 == chess_none) {  // 000221
                    return sleep2; //眠二
                } else if (leftColor1 == myColor && leftColor2 == myColor) { //左边是自己棋子 220221
                    return rush4; //冲四
                } else if (leftColor1 == myColor || leftColor2 == chess_none) { //左边是自己的棋子 020221 或 200221
                    return sleep3; //眠三
                }
            }
        }
    }

    //一连，该棋就是要下的棋子
    if (count ==  1) {
        //记录一连两边各四个位置
        int leftColor1 = chess[left - 1];
        int leftColor2 = chess[left - 2];
        int leftColor3 = chess[left - 3];
        int rightColor1 = chess[right + 1];
        int rightColor2 = chess[right + 2];
        int rightColor3 = chess[right + 3];

        //在该点落子能连成冲四 20222 或 22202
        if (leftColor == chess_none && leftColor1 == myColor &&
                leftColor2 == myColor && leftColor3 == myColor) {
            return rush4;
        }
        if (rightColor == chess_none && rightColor1 == myColor &&
                rightColor2 == myColor && rightColor3 == myColor) {
            return rush4;
        }

        //022020 冲三
        if (leftColor == chess_none && leftColor1 == myColor &&
                leftColor2 == myColor && leftColor3 == chess_none && rightColor == chess_none) {
            return rush3;
        }
        if (rightColor == chess_none && rightColor1 == myColor &&
                rightColor2 == myColor && rightColor3 == chess_none && leftColor == chess_none) {
            return rush3;
        }

        //122020 眠三
        if (leftColor == chess_none && leftColor1 == myColor &&
                leftColor2 == myColor && leftColor3 == opponentColor && rightColor == chess_none) {
            return sleep3;
        }
        if (rightColor == chess_none && rightColor1 == myColor &&
                rightColor2 == myColor && rightColor3 == opponentColor && leftColor == chess_none) {
            return sleep3;
        }

        //眠三22002
        if (leftColor == chess_none && leftColor1 == chess_none &&
                leftColor2 == myColor && leftColor3 == myColor) {
            return sleep3;
        }
        if (rightColor == chess_none && rightColor1 == chess_none &&
                rightColor2 == myColor && rightColor3 == myColor) {
            return sleep3;
        }

        //眠三 20202
        if (leftColor == chess_none && leftColor1 == myColor &&
                leftColor2 == chess_none &&  leftColor3 == myColor) {
            return sleep3;
        }
        if (rightColor == chess_none && rightColor1 == myColor &&
                rightColor2 == chess_none &&  rightColor3 == myColor) {
            return sleep3;
        }

        //冲二 002020
        if (leftColor == chess_none && leftColor1 == myColor &&
                leftColor2 == chess_none && leftColor3 == chess_none && rightColor == chess_none) {
            return rush2;
        }
        if (rightColor == chess_none && rightColor1 == myColor &&
                rightColor2 == chess_none && rightColor3 == chess_none && leftColor == chess_none) {
            return rush2;
        }

        //冲二 020020
        if (leftColor == chess_none && leftColor1 == chess_none &&
                leftColor2 == myColor && leftColor3 == chess_none && rightColor == chess_none) {
            return rush2;
        }
        if (rightColor == chess_none && rightColor1 == chess_none &&
                rightColor2 == myColor && rightColor3 == chess_none && leftColor == chess_none) {
            return rush2;
        }
    }
    return safety;
}

/**
 * @brief judgeColor 判断对手玩家的棋子颜色
 * @param color 当前玩家的棋子颜色
 * @return
 */
int ChessFromJudge::judgeColor(const int color)
{
    if (color == chess_black) {
        return chess_white;
    }
    return chess_black;
}

/**
 * @brief collectChess 收集一个方向上的9个棋子
 * @param chess 存储收集好的棋子数组
 * @param chessState 棋盘状态
 * @param color 棋子颜色
 * @param position 位置
 * @param direction 方向
 */
void ChessFromJudge::collectChess(int chess[9], const ChessState chessState,
                                  const int color, const Position position, int direction)
{
    //出界时设置为对手颜色
    int opponentColor = ChessFromJudge::judgeColor(color);

    chess[4] = color; //该数组的中心位置为该用户所持棋子

    switch (direction) {
    case horizontal: //横向棋子
        //左边四个棋子，遇到边界时记为对手棋子
        for (int i = position.first, j = 1; j <= 4; j++) {
            int col = position.second - j;
            if (col < 0) { //出界
                for (; j <= 4; j++) {
                    chess[4 - j] = opponentColor;
                }
                break;
            }
            chess[4 - j] = chessState[i][col];
        }
        //右边四个棋子，遇到边界时记为对手棋子
        for (int i = position.first, j = 1; j <= 4; j++) {
            int col = position.second + j;
            if (col > (line_col - 1)) {
                for (; j <= 4; j++) {
                    chess[4 + j] = opponentColor;
                }
                break;
            }
            chess[4 + j] = chessState[i][col];
        }
        break;
    case vertical: //竖向棋子
        //上边四个棋子，遇到边界记为对手棋子
        for (int j = position.second, i = 1; i <= 4; i++) {
            int row = position.first - i;
            if (row < 0) {
                for (; i <= 4; i++) {
                    chess[4 - i]  = opponentColor;
                }
                break;
            }
            chess[4 - i] = chessState[row][j];
        }
        //下边四个棋子，遇到边界记为对手棋子
        for (int j = position.second, i = 1; i <= 4; i++) {
            int row = position.first + i;
            if (row > (line_row - 1)) {
                for (; i <= 4; i++) {
                    chess[4 + i] = opponentColor;
                }
                break;
            }
            chess[4 + i] = chessState[row][j];
        }
        break;
    case northeast: //右上左下
        //左下方四个棋子
        for (int i = 1, j = 1; i <= 4; j++, i++) {
            int row = position.first + i;
            int col = position.second - j;
            if (row > (line_row - 1) || col < 0) {
                for (; i <= 4; i++) {
                    chess[4 - i] = opponentColor;
                }
                break;
            }
            chess[4 - i] = chessState[row][col];
        }
        //右上方四个棋子
        for (int i = 1, j = 1; i <= 4; i++, j++) {
            int row = position.first - i;
            int col = position.second + j;
            if (row < 0 || col > (line_col - 1)) {
                for (; i <= 4; i++) {
                    chess[4 + i] = opponentColor;
                }
                break;
            }
            chess[4 + i] = chessState[row][col];
        }
        break;
    case northwest: //左上右下
        //左上方四个 棋子
        for (int i = 1, j = 1; i <= 4; i++, j++) {
            int row = position.first - i;
            int col = position.second - j;
            if (row < 0 || col < 0) {
                for (; i <= 4; i++) {
                    chess[4 - i] = opponentColor;
                }
                break;
            }
            chess[4 - i] = chessState[row][col];
        }
        //右下方四个棋子
        for (int i = 1, j = 1; i <= 4; j++, i++) {
            int row = position.first + i;
            int col = position.second + j;
            if (col > (line_col - 1) || row > (line_row - 1)) {
                for (; i <= 4; i++) {
                    chess[4 + i] = opponentColor;
                }
                break;
            }
            chess[4 + i] = chessState[row][col];
        }
        break;
    default:
        break;
    }
}

/**
 * @brief judgeResult 棋局形式判断
 * @param chessState 棋盘状态数组
 * @param chess 最后一次落子数据
 * @return 返回胜利、平局、游戏进行中三种枚举值
 */
ChessResult ChessFromJudge::judgeResult(const ChessState chessState, const Chess chess)
{
    ChessResult result; //胜利形式
    int lastRow = chess.x; //最后一次行号
    int lastCol = chess.y; //最后一次列号
    int color = chess.color; // 记录最后一次落子的颜色
    int count = 0; // 统计连起来的棋子数目
    int left = 0, right = 0, top = 0, button = 0; //记录下该位置的上下左右边界，用来判断出界问题

    //游戏刚开始
    if (lastCol == -1 && lastRow == -1) {
        return ChessResult::playing;
    }

    //判断是否达成平局
    int flag = 0;
    int i = 0, j = 0;
    for (i = 0; i < line_row; i++) {
        for (j = 0; j < line_col; j++) {
            if (chessState[i][j] == 0) { //有位置没落子，不存在平局
                flag = 1;
                break;
            }
        }
        if (flag) { //退出外层循环
            break;
        }
    }
    if (i == line_row && j == line_col) {
        return ChessResult::tie; //平局
    }

    if (color == chess_black) {
        result = ChessResult::black_win;
    } else {
        result = ChessResult::white_win;
    }
    //横向判断
    left  = (lastCol - 4) < 0 ? 0 : (lastCol - 4); //左边出界时记为0
    right = (lastCol + 4) > (line_col - 1) ? (line_col - 1) : (lastCol + 4); //右边出界时为12
    for (i = lastRow, j = left; j <= right; j++) {
        if (chessState[i][j] == color) {
            count++;
            if (count == 5) {
                return result; //五连珠胜利
            }
        } else {
            count = 0; //断开重新计数
        }
    }

    //纵向判断
    count = 0;
    top = (lastRow - 4) < 0 ? 0 : (lastRow - 4); //上边出界时记为0
    button = (lastRow + 4) > (line_row - 1) ? (line_row - 1) : (lastRow + 4); //下边边出界时记为12
    for (i = top, j = lastCol; i <= button; i++) {
        if (chessState[i][j] == color) {
            count++;
            if (count == 5) {
                return result;
            }
        } else {
            count = 0;
        }
    }

    //左下右上判断
    count  = 0;
    top = lastRow - 4; //行的最小值
    right = lastCol + 4; //列的最大值
    if (top < 0 || right > (line_col - 1)) { //右上出界
        if (lastRow < (line_col - 1 - lastCol)) {//判断行先出界还是列先出界
            top = 0; //行出界
            right = lastRow + lastCol; //行几步出界，列就几步
        } else {
            right = line_col - 1;
            top = lastRow - (line_col - 1 - lastCol);//列几步出界，行就几步
        }
    }

    //左下
    button = lastRow + 4;//行最大值
    left = lastCol - 4; //列最小值
    if (left < 0 || button > (line_row - 1)) { //左下出界
        if (lastCol < (line_row - 1 - lastRow)) { //列先出界
            left = 0;
            button = lastRow + lastCol;
        } else {
            button = line_row - 1;
            left = lastCol - (line_row - 1 - lastRow);
        }
    }

    //统计棋子数目
    for (i = top, j = right; i <= button && j >= left; i++, j--) {
        if (chessState[i][j] == color) {
            count++;
            if (count == 5) {
                return result;
            }
        } else {
            count = 0;
        }
    }

    //左上右下判断
    count = 0;
    left = lastCol - 4; //列最小值
    top = lastRow - 4; //行最小值
    if (left < 0 || top < 0) { //左上出界
        if (lastCol < lastRow) { //列先出界
            left = 0;
            top = lastRow - lastCol;
        } else {
            top = 0;
            left = lastCol - lastRow;
        }
    }
    //右下
    right = lastCol + 4; //列最大值
    button  = lastRow + 4; //行最大值
    if (right > (line_col - 1) || button > (line_row - 1)) { //右下出界
        if (lastCol < lastRow) { // 同时加 谁大谁先出界 行先出界
            button = line_row - 1;
            right = lastCol + (line_row - 1 - lastRow);
        } else {
            right = line_col - 1;
            button = lastRow + (line_col - 1 - lastCol);
        }
    }
    //统计棋子数目
    for (i = top, j = left; i <= button && j <= right; i++, j++) {
        if (chessState[i][j] == color) {
            count++;
            if (count == 5) {
                return result;
            }
        } else {
            count = 0;
        }
    }
    return playing; //游戏还在继续
}


