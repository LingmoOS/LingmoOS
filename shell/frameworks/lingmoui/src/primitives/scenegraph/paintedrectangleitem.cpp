/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "paintedrectangleitem.h"

#include <QPainter>
#include <cmath>

PaintedRectangleItem::PaintedRectangleItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
}

void PaintedRectangleItem::setColor(const QColor &color)
{
    m_color = color;
    update();
}

void PaintedRectangleItem::setRadius(qreal radius)
{
    m_radius = radius;
    update();
}

void PaintedRectangleItem::setBorderColor(const QColor &color)
{
    m_borderColor = color;
    update();
}

void PaintedRectangleItem::setBorderWidth(qreal width)
{
    m_borderWidth = width;
    update();
}

void PaintedRectangleItem::paint(QPainter *painter)
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::transparent);

    auto radius = std::min(m_radius, std::min(width(), height()) / 2);
    auto borderWidth = std::floor(m_borderWidth);

    if (borderWidth > 0.0) {
        painter->setBrush(m_borderColor);
        painter->drawRoundedRect(0, 0, width(), height(), radius, radius);
    }

    painter->setBrush(m_color);
    painter->drawRoundedRect(borderWidth, borderWidth, width() - borderWidth * 2, height() - borderWidth * 2, radius, radius);
}

#include "moc_paintedrectangleitem.cpp"
