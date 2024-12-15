// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BTSTARTPAUSE_H
#define BTSTARTPAUSE_H

#include "buttonitem.h"

class BTStartPause : public ButtonItem
{
    Q_OBJECT
public:
    explicit BTStartPause(QGraphicsItem *parent = nullptr);
    ~BTStartPause() override;

    void setButtonStatus(bool status);
    void setGameOverStatus(bool gameover);
    bool getButtonStatus();

    inline qreal & getPosHeight() {return posHeight;}
    inline qreal getFirstGamePosHeight() {return firstGamePosHeight;}

signals:
    void signalGameStart(); //游戏开始信号
    void signalGameStop(); //游戏暂停信号
    void signalGameContinue(); //游戏继续信号

public:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setNotFirstGame() override;

private:
    void buttonFunction() override;

private:
    QPixmap beginPixmap; //开始按钮图片
    QPixmap stopPixmap; //暂停按钮图片
    bool gameOverStatus = false; //游戏结束标志
    qreal posHeight = 0.2314; //按钮位置占整个scene高度比例
    const qreal firstGamePosHeight = 0.2314;  //游戏未开始时按钮位置占整个scene高度比例
    const qreal notFirstGamePosHeight = 0.333;  //游戏开始后按钮位置占整个scene高度比例
};

#endif // BTSTARTPAUSE_H
