// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "buttonitem.h"

#include <QTimer>

ButtonItem::ButtonItem(QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , backgrounePix(DHiDPIHelper::loadNxPixmap(":/resources/function_button/normal.svg"))
    , normalBackgrounePix(DHiDPIHelper::loadNxPixmap(":/resources/function_button/normal.svg"))
    , hoverBackgrounePix(DHiDPIHelper::loadNxPixmap(":/resources/function_button/hover.svg"))
    , PressBackgrounePix(DHiDPIHelper::loadNxPixmap(":/resources/function_button/press.svg"))
{
    setAcceptHoverEvents(true);
}

ButtonItem::~ButtonItem()
{
}

//设置鼠标hover状态图片
void ButtonItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    backgrounePix = hoverBackgrounePix;
    update();
    QGraphicsItem::hoverEnterEvent(event);
}

//设置正常图片
void ButtonItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    pressStatus = false;
    backgrounePix =  normalBackgrounePix;
    update();
    QGraphicsItem::hoverLeaveEvent(event);
}

//设置鼠标点击图片
void ButtonItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() & Qt::LeftButton) {
        event->accept();
        pressStatus = true;
        backgrounePix = PressBackgrounePix;
        update();
    }
}

//鼠标点击释放，设置图片
void ButtonItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() & Qt::LeftButton) {
        pressStatus = false;
        if (contains(event->pos())) {
            backgrounePix =  normalBackgrounePix;
            mouseReleased = !mouseReleased;
            //设置一个1ms的延时,保证按钮状态切换
            QTimer::singleShot(1, this, [ = ] {
                buttonFunction();
            });
        }
        update();
        QGraphicsItem::mouseReleaseEvent(event);
    }
}

void ButtonItem::setElidedText(QString &text, QFontMetrics &fontMetrics, const int textWidth)
{
    if (fontMetrics.width(text) > (textWidth - 8)) { //判断字符串是否超出长度
        setToolTip(text);
        text = fontMetrics.elidedText(text, Qt::ElideRight, textWidth - 16); //超出后截断用省略号显示
    } else {
        setToolTip(nullptr);
    }
}

/**
 * @brief ButtonItem::setFirstGame //设置为新游戏状态
 */
void ButtonItem::setFirstGame(qreal &posHeight, const qreal firstGamePosHeight)
{
    posHeight = firstGamePosHeight;
    firstStartGame = true;
    mouseReleased = true;
    update();
}

