// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "photorecordbtn.h"

#include <DApplicationHelper>

#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QSvgRenderer>
#include <QMouseEvent>

photoRecordBtn::photoRecordBtn(QWidget *parent/* = nullptr*/)
    :QWidget (parent),
      m_bPhoto(true),
      m_bFocus(false),
      m_bPress(false),
      m_recordState(Normal)
{

}



void photoRecordBtn::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
    painter.setBrush(Qt::NoBrush);

    QPainterPath path;
    QRectF rect = this->rect();

    QColor color = m_bFocus ? Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color() : QColor(0, 0, 0, 0.1 * 255);
    painter.setPen(QPen(color, 2));
    painter.setBrush(Qt::NoBrush);
    rect.setTopLeft(QPoint(1, 1));
    rect.setSize(QSize(62, 62));
    painter.drawEllipse(rect);

    rect.setTopLeft(QPoint(4, 4));
    rect.setSize(QSize(56, 56));
    painter.setPen(QPen(Qt::white, 4));
    painter.drawEllipse(rect);

    rect.setTopLeft(QPoint(6, 6));
    rect.setSize(QSize(52, 52));
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(QColor(255, 255, 255, 0.1 * 255)));
    painter.drawEllipse(rect);

    if (!m_bPhoto)
    {
        if (Normal == m_recordState){
            rect.setTopLeft(QPoint(11, 11));
            rect.setSize(QSize(42, 42));
            if (m_bPress) {
                rect.setTopLeft(QPoint(13, 13));
                rect.setSize(QSize(38,38));
            }
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(QColor(0xff,0x57,0x36,0xff)));
            painter.drawEllipse(rect);
        } else if (preRecord == m_recordState){
            rect.setTopLeft(QPoint(20, 20));
            rect.setSize(QSize(24, 24));
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(0, 0, 0, 255 * 0.08));
            painter.drawRoundedRect(rect, 5, 5);

            rect.setTopLeft(QPoint(22, 22));
            rect.setSize(QSize(20,20));
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(QColor(255,255,255,0xff)));
            painter.drawRoundedRect(rect,4,4);
        } else if (Recording == m_recordState){
            rect.setTopLeft(QPoint(11, 11));
            rect.setSize(QSize(42, 42));
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(QColor(255, 255, 255)));
            painter.drawEllipse(rect);

            rect.setTopLeft(QPoint(23, 23));
            rect.setSize(QSize(18,18));
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(QColor(0xff,0x57,0x36,0xff)));
            painter.drawRoundedRect(rect, 4, 4);
        }

    } else {
        rect.setTopLeft(QPoint(11, 11));
        rect.setSize(QSize(42, 42));
        if (m_bPress) {
            rect.setTopLeft(QPoint(13, 13));
            rect.setSize(QSize(38,38));
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(QColor(Qt::white)));
        painter.drawEllipse(rect);
    }
}

void photoRecordBtn::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_bFocus = true;
    update();
}

void photoRecordBtn::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_bFocus = false;
    update();
}

void photoRecordBtn::mousePressEvent(QMouseEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick)
        return; //不响应双击事件

    m_bFocus = false;

    m_bPress = true;

    update();
}

void photoRecordBtn::mouseMoveEvent(QMouseEvent *event)
{
    //解决bug 在按钮中可拖动相机界面，https://pms.uniontech.com/zentao/bug-view-100647.html
    Q_UNUSED(event);
    return;
}

void photoRecordBtn::mouseReleaseEvent(QMouseEvent *event)
{
    m_bPress = false;

    if (event->type() == QEvent::MouseButtonDblClick)
        return; //不响应双击事件

    if (!rect().contains(event->pos()))
        return;

    m_bFocus= true;
    update();
    emit clicked();
}

void photoRecordBtn::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    m_bFocus= true;
    update();
}

void photoRecordBtn::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    m_bFocus= false;
    update();
}
