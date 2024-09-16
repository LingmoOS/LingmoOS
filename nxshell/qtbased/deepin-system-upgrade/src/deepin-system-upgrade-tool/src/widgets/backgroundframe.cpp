// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "backgroundframe.h"

#include <DApplicationHelper>

#include <QResizeEvent>
#include <QPainter>
#include <QDebug>

DWIDGET_USE_NAMESPACE

BackgroundFrame::BackgroundFrame(QWidget *parent, int cornerRadius)
    : QFrame(parent)
    , m_xRadius(cornerRadius)
    , m_yRadius(cornerRadius)
    , m_brush(DApplicationHelper::instance()->palette(this).brush(DPalette::ItemBackground))
{
    m_pen.setStyle(Qt::NoPen);
}

void BackgroundFrame::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
}

void BackgroundFrame::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setPen(m_pen);
    p.setBrush(m_brush);
    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), m_xRadius, m_yRadius);
    QFrame::paintEvent(event);
}

void BackgroundFrame::setRadius(qreal x, qreal y)
{
    m_xRadius = x;
    m_yRadius = y;
}
