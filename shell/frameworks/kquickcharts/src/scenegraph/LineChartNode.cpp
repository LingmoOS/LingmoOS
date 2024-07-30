/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "LineChartNode.h"

#include <QtMath>

#include "LineChartMaterial.h"
#include "LineSegmentNode.h"

static const int MaxPointsInSegment = 6;

qreal calculateNormalizedLineWidth(qreal pixelWidth, const QRectF &rect)
{
    if (qFuzzyIsNull(pixelWidth)) {
        return 0.0;
    }

    // This needs to account for both height and vertical aspect, resulting in
    // a formula similar to `height / (height / width)` which funnily enough
    // simplifies to just width.
    return pixelWidth * 0.5 / rect.width();
}

LineChartNode::LineChartNode()
{
}

LineChartNode::~LineChartNode()
{
}

void LineChartNode::setRect(const QRectF &rect, qreal devicePixelRatio)
{
    if (rect == m_rect) {
        return;
    }

    m_rect = rect;
    m_aspect = m_rect.height() / m_rect.width();

    auto nativeSize = QSizeF(m_rect.width() * devicePixelRatio, m_rect.height() * devicePixelRatio);
    auto diagonal = std::sqrt(nativeSize.width() * nativeSize.width() + nativeSize.height() * nativeSize.height());
    m_smoothing = 1.0 / diagonal;
}

void LineChartNode::setLineWidth(float width)
{
    if (qFuzzyCompare(width, m_lineWidth)) {
        return;
    }

    m_lineWidth = width;
}

void LineChartNode::setLineColor(const QColor &color)
{
    if (m_lineColor == color) {
        return;
    }

    m_lineColor = color;
}

void LineChartNode::setFillColor(const QColor &color)
{
    if (m_fillColor == color) {
        return;
    }

    m_fillColor = color;
}

void LineChartNode::setValues(const QList<QVector2D> &values)
{
    m_values = values;
}

void LineChartNode::updatePoints()
{
    if (m_values.isEmpty()) {
        return;
    }

    auto segmentCount = qCeil(qreal(m_values.count()) / MaxPointsInSegment);

    auto currentX = m_rect.left();
    auto pointStart = 0;
    auto pointsPerSegment = MaxPointsInSegment;

    for (int i = 0; i < segmentCount; ++i) {
        if (i >= childCount()) {
            appendChildNode(new LineSegmentNode{});
        }

        auto segment = static_cast<LineSegmentNode *>(childAtIndex(i));

        auto segmentPoints = m_values.mid(pointStart, pointsPerSegment);
        pointStart += pointsPerSegment;

        auto segmentWidth = segmentPoints.last().x() - currentX;
        auto rect = QRectF(currentX, m_rect.top(), segmentWidth, m_rect.height());

        segment->setRect(rect);
        segment->setAspect(segmentWidth / m_rect.width(), m_aspect);
        segment->setSmoothing(m_smoothing);
        segment->setLineWidth(calculateNormalizedLineWidth(m_lineWidth, m_rect));
        segment->setLineColor(m_lineColor);
        segment->setFillColor(m_fillColor);
        segment->setValues(segmentPoints);
        segment->setFarLeft(m_values.at(std::max(0, pointStart - pointsPerSegment - 1)));
        segment->setFarRight(m_values.at(std::min<int>(m_values.count() - 1, pointStart + 1)));
        segment->update();

        currentX += segmentWidth;
    }

    while (childCount() > segmentCount) {
        auto child = childAtIndex(childCount() - 1);
        removeChildNode(child);
        delete child;
    }
}
