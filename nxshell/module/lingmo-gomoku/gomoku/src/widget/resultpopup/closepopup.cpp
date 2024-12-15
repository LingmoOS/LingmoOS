// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "closepopup.h"

#include <DHiDPIHelper>

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QDebug>

Closepopup::Closepopup(QWidget *parent)
    : DWidget(parent)
    , buttonNormal(DHiDPIHelper::loadNxPixmap(":/resources/resultpopup/normal.svg"))
    , buttonHover(DHiDPIHelper::loadNxPixmap(":/resources/resultpopup/hover.svg"))
    , buttonPress(DHiDPIHelper::loadNxPixmap(":/resources/resultpopup/press.svg"))
{
    setFixedSize(32, 32);
    currentPixmap = buttonNormal;
}

/**
 * @brief Closepopup::mousePressEvent 鼠标点击事件
 * @param event
 */
void Closepopup::mousePressEvent(QMouseEvent *event)
{
    if (event->button() & Qt::LeftButton) {
        currentPixmap = buttonPress;
        DWidget::mouseReleaseEvent(event);
        update();
    }
}

/**
 * @brief Closepopup::mouseReleaseEvent 鼠标释放事件
 * @param event
 */
void Closepopup::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() & Qt::LeftButton) {
        currentPixmap = buttonNormal;
        if (this->rect().contains(event->pos()))
            emit signalCloseClicked();
        DWidget::mouseReleaseEvent(event);
        update();
    }
}
/**
 * @brief Closepopup::enterEvent
 * @param event
 */
void Closepopup::enterEvent(QEvent *event)
{
    currentPixmap = buttonHover;
    DWidget::enterEvent(event);
    update();
}

/**
 * @brief Closepopup::leaveEvent
 * @param event
 * 移出事件
 */
void Closepopup::leaveEvent(QEvent *event)
{
    currentPixmap = buttonNormal;
    DWidget::leaveEvent(event);
    update();
}

/**
 * @brief Closepopup::paintEvent 绘图事件
 * @param event
 */
void Closepopup::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.drawPixmap(this->rect(), currentPixmap);
    painter.restore();
    DWidget::paintEvent(event);
}
