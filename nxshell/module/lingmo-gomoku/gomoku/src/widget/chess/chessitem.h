// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHESSITEM_H
#define CHESSITEM_H

#include <DHiDPIHelper>

#include <QGraphicsItem>
#include <QtMultimedia/QSound>

DWIDGET_USE_NAMESPACE

class ChessItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    explicit ChessItem(int userChessColor, QGraphicsItem *parent = nullptr);

    //set chess type
    void setCurrentchess(int chesstype);
    int getChessColor();
    void setHasPrintChess();
    void setchessStatus(bool chessstatus);
    bool getchessStatus();

    int getChessPlayer();
    void hoverHandle();

signals:
    void signalCPaintItem(ChessItem *item); //当前绘制的item

public slots:
    void slotGameContinue();
    void slotGameStop();
    void slotGameOver();
    void slotIsAIPlaying(bool AIPlaying);

public:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QRectF boundingRect() const override;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QPixmap chessWhitePixmap; //白色棋子图片
    QPixmap chessBlackPixmap; //黑色棋子图片
    int chessType = 0; //棋子颜色
    int userChessType = 0; //玩家棋子颜色
    bool hoverStatus = false; //hover状态标志
    bool chessStatus = false; //是否有棋子标志
    bool gameStatus = true; //是否暂停
    bool gameOver = false; //游戏结束
    bool isAIPlaying = false; //是否为ai下棋
    bool musicControlStatue = true; //游戏音效控制状态
    bool chessHasPrint = false; //
    bool m_isCurrentItem = false; //用户判断hover是否在此item
};

#endif // chessITEM_H
