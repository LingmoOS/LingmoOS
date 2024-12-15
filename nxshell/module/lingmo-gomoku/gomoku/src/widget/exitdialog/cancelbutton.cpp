// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cancelbutton.h"
#include "globaltool.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <DHiDPIHelper>
#include <QDebug>

CancelButton::CancelButton(QWidget *parent)
    : QWidget(parent)
    , buttonNormal(DHiDPIHelper::loadNxPixmap(":/resources/exitdialog/cancel-normal.svg"))
    , buttonHover(DHiDPIHelper::loadNxPixmap(":/resources/exitdialog/cancel-hover.svg"))
    , buttonPress(DHiDPIHelper::loadNxPixmap(":/resources/exitdialog/cancel-press.svg"))
{
    setFixedSize(160, 42); //设置尺寸
    currentPixmap = buttonNormal;
}

/**
 * @brief CancelButton::mousePressEvent
 * @param event
 * 鼠标按压事件
 */
void CancelButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() & Qt::LeftButton) {
        buttonPressed = true;
        currentPixmap = buttonPress;
        DWidget::mousePressEvent(event);
        update();
    }
}

/**
 * @brief CancelButton::mouseReleaseEvent
 * @param event
 * 鼠标释放事件
 */
void CancelButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() & Qt::LeftButton) {
        buttonPressed = false;
        currentPixmap = buttonNormal;
        if (this->rect().contains(event->pos())) {
            emit signalButtonOKClicked();
        }

        DWidget::mouseReleaseEvent(event);
        update();
    }
}

/**
 * @brief CancelButton::enterEvent
 * @param event
 * 移入事件
 */
void CancelButton::enterEvent(QEvent *event)
{
    currentPixmap = buttonHover;
    DWidget::enterEvent(event);
    update();
}

/**
 * @brief CancelButton::leaveEvent
 * @param event
 * 移出事件
 */
void CancelButton::leaveEvent(QEvent *event)
{
    buttonPressed = false;
    currentPixmap = buttonNormal;
    DWidget::leaveEvent(event);
    update();
}

/**
 * @brief CancelButton::paintEvent
 * @param event
 * 重写绘画事件
 */
void CancelButton::paintEvent(QPaintEvent *event)
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
                      - Globaltool::instacne()->getFontSize().dialogOffset); //阻塞弹窗按钮字体大小为17
    painter.setPen("#02412c"); //字体颜色
    if (buttonPressed) {
        painter.setPen("#ffdb9e");
    }
    painter.setFont(font);
    QFontMetrics fontMetrics(font);
    QString text = tr("Keep Playing");
    if (fontMetrics.width(text) > (this->rect().width() - 20)) { //判断字符串是否超出长度
        setToolTip(text);
        text = fontMetrics.elidedText(text, Qt::ElideRight, this->rect().width() - 20);//超出后截断用省略号显示
    }
    painter.drawText(QPointF((rect().width() - fontMetrics.width(text)) / 2,
                             (rect().height() - fontMetrics.height()) / 2 + fontMetrics.ascent() - fontMetrics.descent() / 4), text);

//    painter.drawText(this->rect(), Qt::AlignHCenter | Qt::AlignVCenter, text);
    painter.restore();
    DWidget::paintEvent(event);
}


