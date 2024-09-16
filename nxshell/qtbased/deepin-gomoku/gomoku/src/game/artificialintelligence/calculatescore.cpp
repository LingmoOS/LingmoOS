// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "calculatescore.h"
#include <vector>

CalculateScore::CalculateScore()
{

}

/**
 * @brief getLocationScore 得出该位置的分数
 * @param chessState 棋盘数组
 * @param color 所判断的棋子颜色
 * @param position 判断的位置
 * @return 该位置的分数
 */
int CalculateScore::getLocationScore(const ChessState chessState,
                                     const int color, const Position position)
{
    if (chessState[position.first][position.second]) { //该位置存在棋子
        return SCORE_SELECTED; // 没有任何威胁
    }

    std::unordered_map<ChessFrom, int> locationSituation; //位置形势，记录每个位置上棋局形式数量

    //计算四个方向的棋子，0为横向、1为竖向、2为右上左下，3为左上右下
    for (int  direction = horizontal; direction <= northwest; direction++) {
        ChessFrom type; // 每个方向上的棋局形势
        int chess[9] = {0}; //存储9个位置的棋子

        //收集该方向上的9个棋子
        ChessFromJudge::collectChess(chess, chessState, color, position, direction);
        //获取某个方向上的棋局类型
        type = ChessFromJudge::judgeChessFrom(chess);

        //统计出每个位置上的棋局类型
        switch (type) {
        case ChessFrom::lian5:
            locationSituation[ChessFrom::lian5]++;
            break;
        case ChessFrom::alive4:
            locationSituation[ChessFrom::alive4]++;
            break;
        case ChessFrom::sleep4:
            locationSituation[ChessFrom::sleep4]++;
            break;
        case ChessFrom::rush4:
            locationSituation[ChessFrom::rush4]++;
            break;
        case ChessFrom::alive3:
            locationSituation[ChessFrom::alive3]++;
            break;
        case ChessFrom::rush3:
            locationSituation[ChessFrom::rush3]++;
            break;
        case ChessFrom::sleep3:
            locationSituation[ChessFrom::sleep3]++;
            break;
        case ChessFrom::alive2:
            locationSituation[ChessFrom::alive2]++;
            break;
        case ChessFrom::rush2:
            locationSituation[ChessFrom::rush2]++;
            break;
        case ChessFrom::sleep2:
            locationSituation[ChessFrom::sleep2]++;
            break;
        case ChessFrom::safety:
            locationSituation[ChessFrom::safety]++;
            break;
        }
    }

    //根据统计好的棋局形势返回分数
    return computeScore(locationSituation);
}

/**
 * @brief computeScore计算该位置的分数
 * @param locationSituation 该位置的棋局形势
 * @return
 */
int CalculateScore::computeScore(std::unordered_map<ChessFrom, int> locationSituation)
{
    int sleep_four = locationSituation[ChessFrom::sleep4] + locationSituation[ChessFrom::rush4];
    int alive_three = locationSituation[ChessFrom::alive3] + locationSituation[ChessFrom::rush3];
    int alive_two = locationSituation[ChessFrom::alive2] + locationSituation[ChessFrom::rush2];

    if (locationSituation[ChessFrom::lian5] >= 1) {
        return SCORE_WIN5; // 五连珠
    }
    if (locationSituation[ChessFrom::alive4] >= 1) {
        return SCORE_ALIVE4; //活四
    }
    if (sleep_four >= 2 || (sleep_four >= 1 && alive_three >= 1)) {
        return SCORE_DOUBLESLEEP4_SLEEP4ALIVE3; //双眠四、眠四活三
    }
    if (alive_three >= 2) {
        return SCORE_DOUBLEALIVE3; //双活三
    }
    if (locationSituation[ChessFrom::sleep3] >= 1 && alive_three >= 1) {
        return SCORE_SLEEP3ALIVE3; //眠三活三
    }
    if (locationSituation[ChessFrom::alive3] >= 1 ||
            locationSituation[ChessFrom::sleep4] >= 1 ||
            locationSituation[ChessFrom::rush4] >= 1) {
        return SCORE_ALIVE3_SLEEP4_RUSH4; //活三、眠四、冲四
    }
    if (locationSituation[ChessFrom::rush3] >= 1) {
        return SCORE_RUSH3; //冲三
    }
    if (alive_two >= 2) {
        return SCORE_DOUBLEALIVE2; //双活二
    }
    if (locationSituation[ChessFrom::alive2] >= 1) {
        return SCORE_ALIVE2; //活二
    }
    if (locationSituation[ChessFrom::sleep3] >= 1) {
        return SCORE_SLEEP3; //眠三
    }
    if (locationSituation[ChessFrom::rush2] >= 1) {
        return SCORE_RUSH2; //冲二
    }
    if (locationSituation[ChessFrom::sleep2] >= 1) {
        return SCORE_SLEEP2; //眠二
    }
    return SCORE_SAFETY; //安全没有威胁
}

/**
 * @brief getBestLocation 根据分数得出最佳落子位置
 * @param AIScore AI分数数组
 * @param userScore 用户分数数组
 * @return
 */
Position CalculateScore::getBestLocation(const int AIScore[line_row][line_col],
                                         const int opponentScore[line_row][line_col])
{
    int AIMaxScore = 0; //AI最大分数
    int userMaxScore = 0; //用户最大分数

    std::vector<Position> AIMaxPositions; //存储AI评分最高的位置
    std::vector<Position> opponentMaxPositions; //存储用户评分最高的位置
    std::vector<Position> allMaxPosition; //双方都是最大值的位置

    //ai数组中的最大值，最大值可能有多个
    for (int i = 0; i < line_row; i++) {
        for (int j = 0; j < line_col; j++) {
            if (AIScore[i][j] == AIMaxScore) { //将最大值的位置全部存储
                AIMaxPositions.push_back(std::make_pair(i, j));
            }
            if (AIScore[i][j] > AIMaxScore) {
                AIMaxScore = AIScore[i][j];
                AIMaxPositions.clear(); //有更大值就将前面的全清理出去
                AIMaxPositions.push_back(std::make_pair(i, j));
            }
        }
    }

    //用户数组中的最大值
    for (int i = 0; i < line_row; i++) {
        for (int j = 0; j < line_col; j++) {
            if (opponentScore[i][j] == userMaxScore) { //将最大值的位置全部存储
                opponentMaxPositions.push_back(std::make_pair(i, j));
            }
            if (opponentScore[i][j] > userMaxScore) {
                userMaxScore = opponentScore[i][j];
                opponentMaxPositions.clear(); //有更大值就将前面的全清理出去
                opponentMaxPositions.push_back(std::make_pair(i, j));
            }
        }
    }

    //由AI和用户的最大值判断是进攻还是防守，最佳落子位置
    if (AIMaxScore >= userMaxScore) { //进攻
        int tempScore = 0;
        for (auto it : AIMaxPositions) {
            if (opponentScore[it.first][it.second] == tempScore) { //从AI的最大值中挑选用户也最大的位置
                allMaxPosition.push_back(it);
            }
            if (opponentScore[it.first][it.second] > tempScore) {
                tempScore = opponentScore[it.first][it.second];
                allMaxPosition.clear();
                allMaxPosition.push_back(it);
            }
        }
    } else { //防守
        int tempScore = 0;
        for (auto it : opponentMaxPositions) {
            if (AIScore[it.first][it.second] == tempScore) { //从用户的最大值中挑选AI也最大的位置
                allMaxPosition.push_back(it);
            }
            if (AIScore[it.first][it.second] > tempScore) {
                tempScore = AIScore[it.first][it.second];
                allMaxPosition.clear();
                allMaxPosition.push_back(it);
            }
        }
    }

    //判断最大值数目
    //最大值数目为1，返回该值
    if (allMaxPosition.size() == 1)
        return allMaxPosition.front();
    //最大值数目为多，随机选取其中一个
    srand(static_cast<unsigned>(time(nullptr)));
    int index = rand() % static_cast<int>(allMaxPosition.size());
    return allMaxPosition[static_cast<unsigned long>(index)];
}
