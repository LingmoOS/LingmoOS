// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "RoundColorWidget.h"

#include <DStyle>

#include <QColor>
#include <QPainter>
#include <QApplication>
#include <QPainterPath>

DWIDGET_USE_NAMESPACE

RoundColorWidget::RoundColorWidget(const QColor &color, QWidget *parent)
    : DWidget(parent)
    , m_isSelected(false)
    , m_color(color)
{
}

void RoundColorWidget::setSelected(bool selected)
{
    if (m_isSelected == selected)
        return;

    m_isSelected = selected;

    update();
}

void RoundColorWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_isSelected && !m_allnotify) return;
        Q_EMIT clicked();
    }
}

void RoundColorWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setRenderHints(QPainter::Antialiasing);

    int borderWidth = style()->pixelMetric(static_cast<QStyle::PixelMetric>(DStyle::PM_FocusBorderWidth), nullptr, this);

    QRect squareRect = rect();
    if (m_isSelected) {
        //draw select circle
        QPen pen;
        pen.setBrush(QBrush(m_color));
        pen.setWidth(borderWidth);  //pen width
        painter.setPen(pen);
        QRect r = squareRect.adjusted(3, 3, -3, -3);
        painter.drawEllipse(r);
    }

    QPainterPath path;
    QRect r = squareRect.adjusted(5, 5, -5, -5);
    path.addEllipse(r);
    painter.setClipPath(path);
    painter.fillPath(path, QBrush(m_color));
}
