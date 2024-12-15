// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GAMECONTROL_H
#define GAMECONTROL_H

#include "artificialintelligence/artificialintelligence.h"
#include "gamecontrol/checkerboard.h"

#include <QObject>

class GameControl : public QObject
{
    Q_OBJECT
public:
    explicit GameControl(int AIColor, int userColor, QObject *parent = nullptr);
    ~GameControl();

    //开始游戏
    void startGame();
    void setChessColor(int ai, int user);
signals:
    void AIPlayChess(Chess chess); //电脑下棋
    void isAIPlaying(bool AIPlaying); //谁在下棋
    void gameOver(ChessResult result); //返回游戏结束标志

public slots:
    //接受主页面绘制完成棋子的槽函数
    void chessCompleted(Chess chess);

    //重置游戏
    void resetGame();

private:
    int AIColor; //电脑颜色
    int userColor; //用户颜色
    bool AIPlaying; //ai下棋
    bool gameReset = false; //重玩游戏标志
    bool initGameStatus = false; //初始化游戏

    Checkerboard *checkerboard;

    void initGame(); //初始化数组

    //下棋函数，棋局判断和下棋者判断
    void playChess(const Chess chess);


private slots:
    void setAIChess();
};
#endif // GAMECONTROL_H
