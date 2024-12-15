// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "buttonrest.h"
#include "globaltool.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QDebug>
#include <DHiDPIHelper>

Buttonrest::Buttonrest(QWidget *parent)
    : DWidget(parent)
    , winRestNormal(DHiDPIHelper::loadNxPixmap(":/resources/resultpopup/win_rest_normal.svg"))
    , winRestHover(DHiDPIHelper::loadNxPixmap(":/resources/resultpopup/win_rest_hover.svg"))
    , winRestPress(DHiDPIHelper::loadNxPixmap(":/resources/resultpopup/win_rest_press.svg"))
    , failRestNormal(DHiDPIHelper::loadNxPixmap(":/resources/resultpopup/fail_rest_normal.svg"))
    , failRestHover(DHiDPIHelper::loadNxPixmap(":/resources/resultpopup/fail_rest_hover.svg"))
    , failRestPress(DHiDPIHelper::loadNxPixmap(":/resources/resultpopup/fail_rest_press.svg"))
    , mResult(false)
{
    //设置大小
    setFixedSize(160, 42);
}

Buttonrest::~Buttonrest()
{

}

/**
 * @brief Buttonrest::setResult 设置对局结果
 * @param result 结果:true为赢,false为输
 */
void Buttonrest::setResult(bool result)
{
    mResult = result;
    //根据输赢情况设置不同的图片
    if (mResult) {
        currentPixmap = winRestNormal;
    } else {
        currentPixmap = failRestNormal;
    }
}

/**
 * @brief Buttonrest::mousePressEvent 鼠标点击事件
 * @param event
 */
void Buttonrest::mousePressEvent(QMouseEvent *event)
{
    if (event->button() & Qt::LeftButton) {
        buttonPressed = true;
        if (mResult) {
            currentPixmap = winRestPress;
        } else {
            currentPixmap = failRestPress;
        }
        DWidget::mousePressEvent(event);
        update();
    }
}

/**
 * @brief Buttonrest::mouseReleaseEvent 鼠标释放事件
 * @param event
 */
void Buttonrest::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() & Qt::LeftButton) {
        buttonPressed = false;
        if (mResult) {
            currentPixmap = winRestNormal;
        } else {
            currentPixmap = failRestNormal;
        }
        if (this->rect().contains(event->pos()))
            emit signalButtonRestClicked();
        DWidget::mouseReleaseEvent(event);
        update();
    }
}

/**
 * @brief Buttonrest::enterEvent 鼠标进入区域事件
 * @param event
 */
void Buttonrest::enterEvent(QEvent *event)
{
    if (mResult) {
        currentPixmap = winRestHover;
    } else {
        currentPixmap = failRestHover;
    }
    DWidget::enterEvent(event);
    update();
}

/**
 * @brief Buttonrest::leaveEvent 鼠标离开区域事件
 * @param event
 */
void Buttonrest::leaveEvent(QEvent *event)
{
    if (mResult) {
        currentPixmap = winRestNormal;
    } else {
        currentPixmap = failRestNormal;
    }
    DWidget::leaveEvent(event);
    update();
}

/**
 * @brief Buttonrest::paintEvent 绘图事件
 * @param event
 */
void Buttonrest::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.save();
    painter.setPen(Qt::NoPen);
    painter.drawPixmap(this->rect(), currentPixmap);
    QFont font;
    font.setFamily(Globaltool::instacne()->loadFontFamilyFromFiles(":/resources/font/ResourceHanRoundedCN-Bold.ttf"));
    font.setWeight(QFont::Bold);
    font.setPixelSize(Globaltool::instacne()->getFontSize().dialogButton
                      - Globaltool::instacne()->getFontSize().dialogOffset);
    painter.setPen("#353535");
    if (buttonPressed) {
        if (currentPixmap == winRestPress) {
            painter.setPen("#ffdb9e");
        } else if (currentPixmap == failRestPress) {
            painter.setPen("#d2d2d2");
        }
    }
    painter.setFont(font);
    QFontMetrics fontMetrics(font);
    QString text = tr("Have a Rest");
    if (fontMetrics.width(text) > (this->rect().width() - 20)) {
        setToolTip(text);
        text = fontMetrics.elidedText(text, Qt::ElideRight, this->rect().width() - 20);
    }
    painter.drawText(QPointF((rect().width() - fontMetrics.width(text)) / 2,
                             (rect().height() - fontMetrics.height()) / 2 + fontMetrics.ascent() - fontMetrics.descent() / 4), text);
//    painter.drawText(this->rect(), Qt::AlignHCenter | Qt::AlignVCenter, text);
    painter.restore();
    DWidget::paintEvent(event);
}
