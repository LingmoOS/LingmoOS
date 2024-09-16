// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BUTTONITEM_H
#define BUTTONITEM_H

#include "constants.h"
#include "globaltool.h"

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QObject>

#include <DHiDPIHelper>

DWIDGET_USE_NAMESPACE
class ButtonItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    explicit ButtonItem(QGraphicsItem *parent = nullptr);
    ~ButtonItem() override;

    virtual void setFirstGame(qreal &posHeight, const qreal firstGamePosHeight);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void buttonFunction() = 0;
    virtual void setNotFirstGame() = 0;
    void setElidedText(QString &text, QFontMetrics &fontMetrics, const int textWidth);

    bool mouseReleased = true; //鼠标是否释放
    bool pressStatus = false; //鼠标点击状态标志
    bool firstStartGame = true; //是否第一次开始游戏
    //以下位置参数,根据UI图得来
    const qreal buttonPosWidth = 0.74; //功能按钮位置占整个scene宽度比例

    const qreal pixmapPosWidth = 0.147; //按钮图标位置占整个scene宽度比例
    const qreal pixmapPosHeight = 0.3125; //按钮图标位置占整个scene高度比例
    const qreal textPosWidth = 0.372; //按钮文字位置占整个scene宽度比例
    const qreal textPosHeight = 0.646; //按钮文字位置占整个scene高度比例
    QPixmap backgrounePix; //需要显示的背景图片
private:
    QPixmap normalBackgrounePix; //正常的背景图片
    QPixmap hoverBackgrounePix; //hover状态的背景图片
    QPixmap PressBackgrounePix; //press状态的背景图片
};

#endif // BUTTONITEM_H
