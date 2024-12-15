// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHECKERBOARDSCENE_H
#define CHECKERBOARDSCENE_H

#include "chess/chessitem.h"
#include "checkerboarditem.h"
#include "buttonfunction/btstartpause.h"
#include "buttonfunction/btreplay.h"
#include "buttonfunction/btmusiccontrol.h"
#include "playingscreen/playingscreen.h"
#include "selectchess/selectchess.h"
#include "constants.h"
#include "gamecontrol/gamecontrol.h"

#include <QGraphicsScene>

class CheckerboardScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit CheckerboardScene(qreal x, qreal y, qreal width, qreal height, QObject *parent = nullptr);
    ~CheckerboardScene() override;

    int getUserChessColor();
    bool getMusicPlay();
    void stopGAme();
    void startGame();
    void setSelectChess(int userColor, int aiColor);
    void selsectChessOK();
    void replayFunction();
    void setStartPauseStatus();

    inline void setGameState(GameState gameState) {m_gameState = gameState;}
    inline GameState getGameState() {return m_gameState;}

signals:
    void signalCurrentPoint(Chess chess); //发送当前棋子坐标
    void signalIsAIPlaying(bool AIPlaying);
    void signalGameContinue();
    void signalGameOver();
    void signalRestGame();
    void signalCloSelectPopup();
    void signalSelectChessPopup();
    void signalReplayFunction();
    void signalPopupResult(ChessResult result);

    void signalNewGame();

public slots:
    void slotPaintAIChess(Chess chess);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    void initGame();
    void initChess();
    void removeButtonFunction();
    void addButtonFunction();
    void initPlayingScreen();
    void setAIChess(Chess chess);

private slots:
    void slotGameStart();
    void slotGameStop();
    void slotGameContinue();
    void slotGameOver(ChessResult result);
    void slotCPaintItem(ChessItem *cItem);

private:
    QSound *playChessSound = nullptr;
    QVector<QVector<ChessItem *>> chessItemList{}; //棋子数组
    bool chessHasPaint[13][13] = {{false}}; //棋子坐标数组,保存每个位置是否绘制棋子
    CheckerboardItem *cbitem = nullptr; //棋盘item
    BTStartPause *buttonStartPause = nullptr; //开始、暂停按钮
    BTReplay *buttonReplay = nullptr; //重玩按钮
    BTMusicControl *buttonMusicControl = nullptr; //音乐控制按钮
    PlayingScreen *playingScreen = nullptr; //对局显示
    int userChessColor = 0; //用户棋子颜色
    int aiChessColor = 0; //ai棋子颜色
    GameControl *gameControl = nullptr; //游戏控制
//    int userChessType = chess_black; //棋子颜色
    int clickPosRow = -1; //点击的行
    int clickPosCol = -1; // 点击的列
    bool musicControlStatus = true; //是否可以播放音乐
    bool gameStatus = false; //游戏状态
//    bool seleceChessPopup = true; //是否弹出选择棋子弹窗
    bool playChess = false; //是否是AI下棋
    bool AIChessStatus = false; //暂停游戏时,AI是否下棋
    bool gameReplay = false; //游戏是否重玩
    Chess AIChess; //保存当前棋子位置
    GameState m_gameState = GameState::gameStart; //是否是新游戏的标志

    //以下位置参数,根据UI图得来
    const qreal lefttopChessPosWidth = 0.02; //左上角第一个棋子位置占整个scene宽度比例
    const qreal lefttopChessPosHeight = 0.007; //左上角第一个棋子位置占整个scene高度比例
    const int chessOffset = 83; //棋子位置偏移量
};

#endif // CHECKERBOARDSCENE_H
