// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "bordermaskwidget.h"

#include <QPainter>
#include <QDebug>

BorderMaskWidget::BorderMaskWidget(QWidget *parent)
    : DBlurEffectWidget(parent)
    , m_backColor(Qt::white)
    , m_radius(18)
    , m_outerBorderColor(Qt::white)
    , m_outerBorderWidth(1)
    , m_innerBorderColor(Qt::white)
    , m_innerBorderWidth(1)
{
    setMaskColor(m_backColor);
    setMaskAlpha(1 * 255);
    setBlurRectXRadius(m_radius);
    setBlurRectYRadius(m_radius);
    setBlendMode(DBlurEffectWidget::BehindWindowBlend);
}

void BorderMaskWidget::paintEvent(QPaintEvent *e)
{
    DBlurEffectWidget::paintEvent(e);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 外边框线
    QPen pen;
    pen.setColor(m_outerBorderColor);
    pen.setWidth(m_outerBorderWidth);
    painter.setPen(pen);
    painter.drawRoundedRect(rect(), m_radius, m_radius);

    const int innerRadius = (m_radius - m_outerBorderWidth) > 0 ? m_radius - m_outerBorderWidth : 0;

    // 内边框线
    pen.setColor(m_innerBorderColor);
    pen.setWidth(m_innerBorderWidth);
    painter.setPen(pen);
    painter.drawRoundedRect(rect().marginsRemoved(QMargins(m_outerBorderWidth,
                                                           m_outerBorderWidth,
                                                           m_outerBorderWidth,
                                                           m_outerBorderWidth)),
                            innerRadius,
                            innerRadius);
}

void BorderMaskWidget::setOuterBorderColor(const QColor &borderColor)
{
    if (m_outerBorderColor != borderColor) {
        m_outerBorderColor = borderColor;
        update();
    }
}

void BorderMaskWidget::setOuterBorderWidth(int borderWidth)
{
    if (m_outerBorderWidth != borderWidth) {
        m_outerBorderWidth = borderWidth;
        update();
    }
}

void BorderMaskWidget::setInnerBorderColor(const QColor &borderColor)
{
    if (m_innerBorderColor != borderColor) {
        m_innerBorderColor = borderColor;
        update();
    }
}

void BorderMaskWidget::setInnerBorderWidth(int borderWidth)
{
    if (m_innerBorderWidth != borderWidth) {
        m_innerBorderWidth = borderWidth;
        update();
    }
}

void BorderMaskWidget::setBackgroundColor(const QColor &backColor)
{
    if (m_backColor != backColor) {
        m_backColor = backColor;
        setMaskColor(m_backColor);
        setMaskAlpha(m_backColor.alphaF() * 255);
        update();
    }
}

void BorderMaskWidget::setRadius(int radius)
{
    if (m_radius != radius) {
        m_radius = radius;
        setBlurRectXRadius(m_radius);
        setBlurRectYRadius(m_radius);
        update();
    }
}
