// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "selectbutton.h"

#include <DHiDPIHelper>

#include <QPainter>
#include <QPainterPath>
#include <QDebug>

Selectbutton::Selectbutton(QWidget *parent)
    : QPushButton(parent)
    , selectNormal(DHiDPIHelper::loadNxPixmap(":/resources/chessselected/radio_unchecked.svg"))
    , selectHover(DHiDPIHelper::loadNxPixmap(":/resources/chessselected/radio_hover.svg"))
    , selectPress(DHiDPIHelper::loadNxPixmap(":/resources/chessselected/radio_checked.svg"))
    , currentStatus(selectNormal)
{
    setFixedSize(24, 24);
    //可选
    setCheckable(true);
    //互斥
    setAutoExclusive(true);
}

/**
 * @brief Selectbutton::enterEvent 鼠标进入区域事件
 * @param event
 */
void Selectbutton::enterEvent(QEvent *event)
{
    currentStatus = selectHover;
    DWidget::enterEvent(event);
    update();
}

/**
 * @brief Selectbutton::leaveEvent 鼠标离开区域事件
 * @param event
 */
void Selectbutton::leaveEvent(QEvent *event)
{
    //如果已经checked, 则设置为press状态
    if (pressStatus) {
        currentStatus = selectPress;
    } else {
        currentStatus = selectNormal;
    }
    DWidget::leaveEvent(event);
    update();
}

/**
 * @brief Selectbutton::paintEvent 绘图函数
 * @param event
 */
void Selectbutton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.save();
    painter.setPen(Qt::NoPen);
    //如果被选择,设置选择状态图片
    if (isChecked()) {
        painter.drawPixmap(this->rect(), selectPress);
    } else {
        painter.drawPixmap(this->rect(), currentStatus);
    }
    painter.restore();
    DWidget::paintEvent(event);
}
