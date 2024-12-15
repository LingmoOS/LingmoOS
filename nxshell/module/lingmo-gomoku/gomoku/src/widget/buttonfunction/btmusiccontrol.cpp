// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "btmusiccontrol.h"

BTMusicControl::BTMusicControl(QGraphicsItem *parent)
    : ButtonItem(parent)
    , voicePixmap(DHiDPIHelper::loadNxPixmap(":/resources/icon/voice.svg"))
    , voiceOffPixmap(DHiDPIHelper::loadNxPixmap(":/resources/icon/voiceoff.svg"))
{
}

BTMusicControl::~BTMusicControl()
{

}

QRectF BTMusicControl::boundingRect() const
{
    //设置按钮位置,并根据图片大小设置rect大小
    return QRectF(this->scene()->width() * buttonPosWidth,
                  this->scene()->height() * buttonMusicControlPosHeight,
                  204,
                  64);
}

void BTMusicControl::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    qreal rectX = this->boundingRect().x(); //矩形起始位置X
    qreal rectY = this->boundingRect().y(); //矩形起始位置Y
    qreal rectWidth = this->boundingRect().width(); //矩形宽度
    qreal rectHeight = this->boundingRect().height(); //矩形高度
    painter->setRenderHint(QPainter::Antialiasing);

    painter->save();
    painter->setPen(Qt::NoPen);
    painter->drawPixmap(QPointF(rectX, rectY), backgrounePix);
    painter->restore();

    painter->save();
    painter->setPen(Qt::NoPen);
    //声音控制图片
    if (mouseReleased) {
        painter->drawPixmap(QPointF(rectX + rectWidth * pixmapPosWidth,
                                    rectY + rectHeight * pixmapPosHeight),
                            voicePixmap);
    } else {
        painter->drawPixmap(QPointF(rectX + rectWidth * pixmapPosWidth,
                                    rectY + rectHeight * pixmapPosHeight),
                            voiceOffPixmap);
    }
    painter->restore();

    painter->save();
    QFont font;

    font.setFamily(Globaltool::instacne()->loadFontFamilyFromFiles(":/resources/font/ResourceHanRoundedCN-Bold.ttf"));
    font.setWeight(QFont::Black);
    font.setPixelSize(Globaltool::instacne()->getFontSize().functionButton);
    font.setBold(true);
    painter->setFont(font);
    if (pressStatus) {
        painter->setPen(QColor("#ffdb9e"));
    } else {
        painter->setPen(QColor("#024526"));
    }

    QFontMetrics fontMetrics(font);
    QString musicText;
    int musicTextWidth = static_cast<int>(rectWidth - (rectWidth * musicTextPosWidth));

    musicText =  mouseReleased ? tr("Sound Off") : tr("Sound On");
    setElidedText(musicText, fontMetrics, musicTextWidth);
    painter->drawText(QPointF(rectX + rectWidth * musicTextPosWidth,
                              rectY + rectHeight * textPosHeight),
                      musicText);
    painter->restore();
}

/**
 * @brief BTReplay::setNotFirstGame 设置是否第一次开始游戏
 */
void BTMusicControl::setNotFirstGame()
{
//    mouseReleased = true;
    buttonFunction();
    update();
}

//按钮功能
void BTMusicControl::buttonFunction()
{
    //控制音乐
    emit signalMusic(mouseReleased);
}
