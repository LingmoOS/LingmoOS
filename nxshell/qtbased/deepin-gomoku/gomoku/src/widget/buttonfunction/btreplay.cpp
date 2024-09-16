// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "btreplay.h"

BTReplay::BTReplay(QGraphicsItem *parent)
    : ButtonItem(parent)
    , replayPixmap(DHiDPIHelper::loadNxPixmap(":/resources/icon/replay.svg"))
{
    posHeight = firstGamePosHeight;
}

BTReplay::~BTReplay()
{

}

QRectF BTReplay::boundingRect() const
{
    //设置按钮位置,并根据图片大小设置rect大小
    return QRectF(this->scene()->width() * buttonPosWidth,
                  this->scene()->height() * posHeight,
                  204,
                  64);
}

void BTReplay::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    qreal rectX = this->boundingRect().x(); //矩形起始位置X
    qreal rectY = this->boundingRect().y(); //矩形起始位置Y
    qreal rectWidth = this->boundingRect().width(); //矩形宽度
    qreal rectHeight = this->boundingRect().height(); //矩形高度

    painter->setRenderHint(QPainter::Antialiasing);

    //首次开始游戏,设置不同的背景
    if (firstStartGame) {
        painter->save();
        painter->setPen(Qt::red);
        painter->setBrush(Qt::red);
        painter->setOpacity(0.4); //设置图片透明度
        painter->drawPixmap(QPointF(rectX, rectY),
                            DHiDPIHelper::loadNxPixmap(":/resources/function_button/normal.svg"));
        painter->restore();
    } else {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->drawPixmap(QPointF(rectX, rectY), backgrounePix);
        painter->restore();
    }

    painter->save();
    painter->setPen(Qt::NoPen);
    //首次开始游戏
    if (firstStartGame)
        painter->setOpacity(0.4); //设置图片透明度
    painter->drawPixmap(QPointF(rectX + rectWidth * pixmapPosWidth,
                                rectY + rectHeight * pixmapPosHeight),
                        replayPixmap);
    painter->restore();

    painter->save();
    QFont font;
    font.setFamily(Globaltool::instacne()->instacne()->loadFontFamilyFromFiles(":/resources/font/ResourceHanRoundedCN-Bold.ttf"));
    font.setWeight(QFont::Black);
    font.setPixelSize(Globaltool::instacne()->getFontSize().functionButton);
    font.setBold(true);
    painter->setFont(font);
    if (!firstStartGame && pressStatus) {
        painter->setPen(QColor("#ffdb9e"));
    } else {
        painter->setPen(QColor("#024526"));
    }

    QFontMetrics fontMetrics(font);
    QString replayText = tr("New Game");
    int musicTextWidth = static_cast<int>(rectWidth - (rectWidth * textPosWidth));
    setElidedText(replayText, fontMetrics, musicTextWidth);

    //首次开始游戏
    if (firstStartGame)
        painter->setOpacity(0.4); //设置图片透明度
    painter->drawText(QPointF(rectX + rectWidth * textPosWidth,
                              rectY + rectHeight * textPosHeight),
                      replayText);
    painter->restore();
}

/**
 * @brief BTReplay::setButtonState 重玩按钮禁用
 * @param state 重玩按钮的状态：是否禁止点击
 */
void BTReplay::setButtonState(bool state)
{
    firstStartGame = state;
    update();
}

//按钮功能
void BTReplay::buttonFunction()
{
    if (!firstStartGame) {
        //重玩功能
        emit signalbuttonReplay();
    }
}

/**
 * @brief BTReplay::setNotFirstGame 设置是否第一次开始游戏
 */
void BTReplay::setNotFirstGame()
{
    if (qAbs(posHeight - notFirstGamePosHeight) >= (1e-6))
        posHeight = notFirstGamePosHeight;
    firstStartGame = false;
    mouseReleased = false;
    update();
}

