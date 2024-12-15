// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BTREPLAY_H
#define BTREPLAY_H

#include "buttonitem.h"

class BTReplay : public ButtonItem
{
    Q_OBJECT
public:
    explicit BTReplay(QGraphicsItem *parent = nullptr);
    ~BTReplay() override;

public:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    inline qreal & getPosHeight() {return posHeight;}
    inline qreal getFirstGamePosHeight() {return firstGamePosHeight;}

    void setButtonState(bool state);

    void setNotFirstGame() override;

signals:
    void signalbuttonReplay(); //游戏重玩信号

private:
    void buttonFunction() override;

private:
    QPixmap replayPixmap; //重玩按钮图片
    qreal posHeight = 0.3425; //按钮位置占整个scene高度比例
    const qreal firstGamePosHeight = 0.3425;  //游戏未开始时按钮位置占整个scene高度比例
    const qreal notFirstGamePosHeight = 0.444; //重玩功能按钮位置占整个scene高度比例
};

#endif // BTREPLAY_H
