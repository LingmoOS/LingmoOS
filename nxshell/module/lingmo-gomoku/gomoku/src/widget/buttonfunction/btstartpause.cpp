// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "btstartpause.h"

BTStartPause::BTStartPause(QGraphicsItem *parent)
    : ButtonItem(parent)
    , beginPixmap(DHiDPIHelper::loadNxPixmap(":/resources/icon/begin.svg"))
    , stopPixmap(DHiDPIHelper::loadNxPixmap(":/resources/icon/stop.svg"))
{
    posHeight = firstGamePosHeight;
}

BTStartPause::~BTStartPause()
{
}

/**
 * @brief BTStartPause::setStopStatus 设置按钮状态
 */
void BTStartPause::setButtonStatus(bool status)
{
    //修改按钮图片
    mouseReleased = status;
    if (!firstStartGame) {
        //更新功能
        buttonFunction();
    }
    update();
}

/**
 * @brief BTStartPause::setGameOverStatus 设置游戏结束标志
 */
void BTStartPause::setGameOverStatus(bool gameover)
{
    gameOverStatus = gameover;
    if (!gameOverStatus) {
        firstStartGame = true;
    }
    update();
}

/**
 * @brief BTStartPause::getButtonStatus 获取按钮当前状态
 * @return
 */
bool BTStartPause::getButtonStatus()
{
    return mouseReleased;
}

/**
 * @brief BTStartPause::setFirstGameStatus 设置是否第一次开始游戏
 */
void BTStartPause::setNotFirstGame()
{
    if (qAbs(posHeight - notFirstGamePosHeight) >= (1e-6))
        posHeight = notFirstGamePosHeight;
    firstStartGame = false;
    mouseReleased = false;
    buttonFunction();
    update();
}

QRectF BTStartPause::boundingRect() const
{
    //设置按钮位置,并根据图片大小设置rect大小
    return QRectF(this->scene()->width() * buttonPosWidth,
                  this->scene()->height() * posHeight,
                  204,
                  64);
}

void BTStartPause::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    qreal rectX = this->boundingRect().x(); //矩形起始位置X
    qreal rectY = this->boundingRect().y(); //矩形起始位置Y
    qreal rectWidth = this->boundingRect().width(); //矩形宽度
    qreal rectHeight = this->boundingRect().height(); //矩形高度

    QFont font;
    font.setFamily(Globaltool::instacne()->loadFontFamilyFromFiles(":/resources/font/ResourceHanRoundedCN-Bold.ttf"));
    font.setWeight(QFont::Black);
    font.setPixelSize(Globaltool::instacne()->getFontSize().functionButton);
    font.setBold(true);

    painter->setRenderHint(QPainter::Antialiasing);

    QFontMetrics fontMetrics(font);
    QString replayText;
    int startTextWidth = static_cast<int>(rectWidth - (rectWidth * textPosWidth));

    if (gameOverStatus) {
        replayText = tr("Start");
        painter->save();
        painter->drawPixmap(QPointF(rectX, rectY),
                            DHiDPIHelper::loadNxPixmap(":/resources/function_button/normal.svg"));
        //图片或文字位置通过起始位置+偏移量的方式设置
        painter->drawPixmap(QPointF(rectX + rectWidth *  pixmapPosWidth, //起始X + 向左的偏移量
                                    rectY + rectHeight * pixmapPosHeight), //起始Y + 向下的偏移量
                            beginPixmap);
        painter->setFont(font);
        painter->setPen(QColor("#024526"));
    } else {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->drawPixmap(QPointF(rectX, rectY), backgrounePix);
        painter->restore();

        painter->save();
        painter->setPen(Qt::NoPen);
        if (firstStartGame) {
            painter->drawPixmap(QPointF(rectX + rectWidth * pixmapPosWidth,
                                        rectY + rectHeight * pixmapPosHeight),
                                beginPixmap);
        } else {
            if (mouseReleased) {
                painter->drawPixmap(QPointF(rectX + rectWidth * pixmapPosWidth,
                                            rectY + rectHeight * pixmapPosHeight),
                                    beginPixmap);
            } else {
                painter->drawPixmap(QPointF(rectX + rectWidth * pixmapPosWidth,
                                            rectY + rectHeight * pixmapPosHeight),
                                    stopPixmap);
            }
        }
        painter->restore();

        painter->save();
        painter->setFont(font);
        if (pressStatus) {
            painter->setPen(QColor("#ffdb9e"));
        } else {
            painter->setPen(QColor("#024526"));
        }

        if (firstStartGame) {
            replayText = tr("Start");
        } else {
            replayText = mouseReleased ? tr("Continue") : tr("Pause");
        }
    }
    setElidedText(replayText, fontMetrics, startTextWidth);
    painter->drawText(QPointF(rectX + rectWidth * textPosWidth,
                              rectY + rectHeight * textPosHeight),
                      replayText);
    painter->restore();
}

//按钮功能
void BTStartPause::buttonFunction()
{
    if (!firstStartGame) {
        //开始暂停功能
        if (mouseReleased) {
            //暂停游戏
            emit signalGameStop();
        } else {
            //继续游戏
            emit signalGameContinue();
        }
    } else {
        //开始游戏
        emit signalGameStart();
    }
}
