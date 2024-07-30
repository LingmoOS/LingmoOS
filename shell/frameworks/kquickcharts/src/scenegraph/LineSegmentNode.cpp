/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "LineSegmentNode.h"

#include <QSGGeometry>

#include "LineChartMaterial.h"

constexpr int MaxPointsSize = (6 + 8) * 2;

struct LineVertex {
    float position[2];

    float uv[2];

    float lineColor[4];
    float fillColor[4];

    float bounds[2];

    float pointCount;
    float points[MaxPointsSize];

    void set(const QPointF &newPosition,
             const QPointF &newUv,
             const QList<QVector2D> &newPoints,
             const QColor &newLineColor,
             const QColor &newFillColor,
             const QVector2D &newBounds)
    {
        position[0] = newPosition.x();
        position[1] = newPosition.y();

        uv[0] = newUv.x();
        uv[1] = newUv.y();

        lineColor[0] = newLineColor.redF();
        lineColor[1] = newLineColor.greenF();
        lineColor[2] = newLineColor.blueF();
        lineColor[3] = newLineColor.alphaF();

        fillColor[0] = newFillColor.redF();
        fillColor[1] = newFillColor.greenF();
        fillColor[2] = newFillColor.blueF();
        fillColor[3] = newFillColor.alphaF();

        bounds[0] = newBounds.x();
        bounds[1] = newBounds.y();

        setPoints(newPoints);
    }

    void setPoints(const QList<QVector2D> &newPoints)
    {
        memset(points, 0, MaxPointsSize * sizeof(float));

        Q_ASSERT_X(newPoints.size() <= (MaxPointsSize / 2),
                   "LineVertex::setPoints",
                   qPrintable(QStringLiteral("Too many points in new points array: %1").arg(newPoints.size())));

        for (int i = 0; i < newPoints.size(); ++i) {
            points[i * 2 + 0] = newPoints[i].x();
            points[i * 2 + 1] = newPoints[i].y();
        }

        pointCount = newPoints.size();
    }
};

/* clang-format off */
QSGGeometry::Attribute LineAttributes[] = {
    QSGGeometry::Attribute::createWithAttributeType(0, 2, QSGGeometry::FloatType, QSGGeometry::PositionAttribute), // in_position
    QSGGeometry::Attribute::createWithAttributeType(1, 2, QSGGeometry::FloatType, QSGGeometry::TexCoordAttribute), // in_uv

    QSGGeometry::Attribute::createWithAttributeType(2, 4, QSGGeometry::FloatType, QSGGeometry::UnknownAttribute), // in_lineColor
    QSGGeometry::Attribute::createWithAttributeType(3, 4, QSGGeometry::FloatType, QSGGeometry::UnknownAttribute), // in_fillColor

    QSGGeometry::Attribute::createWithAttributeType(4, 2, QSGGeometry::FloatType, QSGGeometry::UnknownAttribute), // in_bounds

    QSGGeometry::Attribute::createWithAttributeType(5, 1, QSGGeometry::FloatType, QSGGeometry::UnknownAttribute), // in_count

    QSGGeometry::Attribute::createWithAttributeType(8, 4, QSGGeometry::FloatType, QSGGeometry::UnknownAttribute), // in_points_0
    QSGGeometry::Attribute::createWithAttributeType(9, 4, QSGGeometry::FloatType, QSGGeometry::UnknownAttribute), // in_points_1
    QSGGeometry::Attribute::createWithAttributeType(10, 4, QSGGeometry::FloatType, QSGGeometry::UnknownAttribute), // in_points_2
    QSGGeometry::Attribute::createWithAttributeType(11, 4, QSGGeometry::FloatType, QSGGeometry::UnknownAttribute), // in_points_3
    QSGGeometry::Attribute::createWithAttributeType(12, 4, QSGGeometry::FloatType, QSGGeometry::UnknownAttribute), // in_points_4
    QSGGeometry::Attribute::createWithAttributeType(13, 4, QSGGeometry::FloatType, QSGGeometry::UnknownAttribute), // in_points_5
    QSGGeometry::Attribute::createWithAttributeType(14, 4, QSGGeometry::FloatType, QSGGeometry::UnknownAttribute), // in_points_6
};
/* clang-format on */

QSGGeometry::AttributeSet LineAttributeSet = {13, sizeof(LineVertex), LineAttributes};

void updateLineGeometry(QSGGeometry *geometry,
                        const QRectF &rect,
                        const QRectF &uvRect,
                        const QList<QVector2D> &points,
                        const QColor &lineColor,
                        const QColor &fillColor,
                        const QVector2D &bounds)
{
    auto vertices = static_cast<LineVertex *>(geometry->vertexData());
    vertices[0].set(rect.topLeft(), uvRect.topLeft(), points, lineColor, fillColor, bounds);
    vertices[1].set(rect.bottomLeft(), uvRect.bottomLeft(), points, lineColor, fillColor, bounds);
    vertices[2].set(rect.topRight(), uvRect.topRight(), points, lineColor, fillColor, bounds);
    vertices[3].set(rect.bottomRight(), uvRect.bottomRight(), points, lineColor, fillColor, bounds);
    geometry->markVertexDataDirty();
}

LineSegmentNode::LineSegmentNode()
    : LineSegmentNode(QRectF{})
{
}

LineSegmentNode::LineSegmentNode(const QRectF &rect)
{
    m_geometry = new QSGGeometry{LineAttributeSet, 4};
    m_geometry->setVertexDataPattern(QSGGeometry::DynamicPattern);

    setGeometry(m_geometry);

    m_rect = rect;

    m_material = new LineChartMaterial{};
    setMaterial(m_material);

    setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial);
}

LineSegmentNode::~LineSegmentNode()
{
}

void LineSegmentNode::setRect(const QRectF &rect)
{
    m_rect = rect;
}

void LineSegmentNode::setAspect(float xAspect, float yAspect)
{
    if (qFuzzyCompare(xAspect, m_xAspect) && qFuzzyCompare(yAspect, m_yAspect)) {
        return;
    }

    m_yAspect = yAspect;
    m_material->aspect = m_yAspect;
    markDirty(QSGNode::DirtyMaterial);

    m_xAspect = xAspect;
}

void LineSegmentNode::setSmoothing(float smoothing)
{
    if (qFuzzyCompare(smoothing, m_smoothing)) {
        return;
    }

    m_smoothing = smoothing;
    m_material->smoothing = m_smoothing;
    markDirty(QSGNode::DirtyMaterial);
}

void LineSegmentNode::setLineWidth(float width)
{
    if (qFuzzyCompare(width, m_lineWidth)) {
        return;
    }

    m_lineWidth = width;
    m_material->lineWidth = m_lineWidth;
    markDirty(QSGNode::DirtyMaterial);
}

void LineSegmentNode::setLineColor(const QColor &color)
{
    m_lineColor = color;
}

void LineSegmentNode::setFillColor(const QColor &color)
{
    m_fillColor = color;
}

void LineSegmentNode::setValues(const QList<QVector2D> &values)
{
    m_values = values;
}

void LineSegmentNode::setFarLeft(const QVector2D &value)
{
    m_farLeft = value;
}

void LineSegmentNode::setFarRight(const QVector2D &value)
{
    m_farRight = value;
}

void LineSegmentNode::update()
{
    if (m_values.isEmpty() || !m_rect.isValid()) {
        updateLineGeometry(m_geometry, QRectF{}, QRectF{}, QList<QVector2D>{}, m_lineColor, m_fillColor, QVector2D{});
        markDirty(QSGNode::DirtyGeometry);
        return;
    }

    QList<QVector2D> points;
    points.reserve(m_values.size() + 8);

    points << QVector2D{0.0, -0.5};
    points << QVector2D{-0.5, -0.5};

    auto min = std::numeric_limits<float>::max();
    auto max = std::numeric_limits<float>::min();

    if (!m_farLeft.isNull()) {
        points << QVector2D(-0.5, m_farLeft.y() * m_yAspect);
        points << QVector2D(((m_farLeft.x() - m_rect.left()) / m_rect.width()) * m_xAspect, m_farLeft.y() * m_yAspect);
        min = std::min(m_farLeft.y() * m_yAspect, min);
        max = std::max(m_farLeft.y() * m_yAspect, max);
    } else {
        points << QVector2D(-0.5, m_values[0].y() * m_yAspect);
    }

    for (auto value : std::as_const(m_values)) {
        auto x = ((value.x() - m_rect.left()) / m_rect.width()) * m_xAspect;
        points << QVector2D(x, value.y() * m_yAspect);
        min = std::min(value.y() * m_yAspect, min);
        max = std::max(value.y() * m_yAspect, max);
    }

    if (!m_farRight.isNull()) {
        points << QVector2D(((m_farRight.x() - m_rect.left()) / m_rect.width()) * m_xAspect, m_farRight.y() * m_yAspect);
        points << QVector2D(1.5, m_farRight.y() * m_yAspect);
        min = std::min(m_farRight.y() * m_yAspect, min);
        max = std::max(m_farRight.y() * m_yAspect, max);
    } else {
        points << QVector2D(1.5, points.last().y());
    }

    points << QVector2D{1.5, -0.5};
    points << QVector2D{0.0, -0.5};

    updateLineGeometry(m_geometry, m_rect, {0.0, 0.0, m_xAspect, 1.0}, points, m_lineColor, m_fillColor, QVector2D{min, max});
    markDirty(QSGNode::DirtyGeometry);
}
