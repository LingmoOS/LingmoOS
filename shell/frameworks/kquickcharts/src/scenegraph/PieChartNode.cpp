/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "PieChartNode.h"

#include <algorithm>

#include <QColor>
#include <QSGGeometry>
#include <cmath>

#include "PieChartMaterial.h"

static const qreal pi = std::acos(-1.0);

inline QVector4D colorToVec4(const QColor &color)
{
    return QVector4D{float(color.redF()), float(color.greenF()), float(color.blueF()), float(color.alphaF())};
}

inline qreal degToRad(qreal deg)
{
    return (deg / 180.0) * pi;
}

PieChartNode::PieChartNode()
    : PieChartNode(QRectF{})
{
}

PieChartNode::PieChartNode(const QRectF &rect)
{
    m_geometry = new QSGGeometry{QSGGeometry::defaultAttributes_TexturedPoint2D(), 4};
    QSGGeometry::updateTexturedRectGeometry(m_geometry, rect, QRectF{0, 0, 1, 1});
    setGeometry(m_geometry);

    m_material = new PieChartMaterial{};
    setMaterial(m_material);

    setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial);
}

PieChartNode::~PieChartNode()
{
}

void PieChartNode::setRect(const QRectF &rect)
{
    if (rect == m_rect) {
        return;
    }

    m_rect = rect;
    QSGGeometry::updateTexturedRectGeometry(m_geometry, m_rect, QRectF{0, 0, 1, 1});
    markDirty(QSGNode::DirtyGeometry);

    auto minDimension = qMin(m_rect.width(), m_rect.height());

    QVector2D aspect{1.0, 1.0};
    aspect.setX(rect.width() / minDimension);
    aspect.setY(rect.height() / minDimension);
    m_material->setAspectRatio(aspect);

    m_material->setInnerRadius(m_innerRadius / minDimension);
    m_material->setOuterRadius(m_outerRadius / minDimension);

    markDirty(QSGNode::DirtyMaterial);
}

void PieChartNode::setInnerRadius(qreal radius)
{
    if (qFuzzyCompare(radius, m_innerRadius)) {
        return;
    }

    m_innerRadius = radius;

    auto minDimension = qMin(m_rect.width(), m_rect.height());
    m_material->setInnerRadius(m_innerRadius / minDimension);

    markDirty(QSGNode::DirtyMaterial);
}

void PieChartNode::setOuterRadius(qreal radius)
{
    if (qFuzzyCompare(radius, m_outerRadius)) {
        return;
    }

    m_outerRadius = radius;

    auto minDimension = qMin(m_rect.width(), m_rect.height());
    m_material->setOuterRadius(m_outerRadius / minDimension);

    markDirty(QSGNode::DirtyMaterial);
}

void PieChartNode::setColors(const QList<QColor> &colors)
{
    m_colors = colors;
    updateSegments();
}

void PieChartNode::setSections(const QList<qreal> &sections)
{
    m_sections = sections;
    updateSegments();
}

void PieChartNode::setBackgroundColor(const QColor &color)
{
    if (color == m_backgroundColor) {
        return;
    }

    m_backgroundColor = color;
    m_material->setBackgroundColor(color);
    markDirty(QSGNode::DirtyMaterial);
}

void PieChartNode::setFromAngle(qreal angle)
{
    if (qFuzzyCompare(angle, m_fromAngle)) {
        return;
    }

    m_fromAngle = angle;
    m_material->setFromAngle(degToRad(angle));
    updateSegments();
}

void PieChartNode::setToAngle(qreal angle)
{
    if (qFuzzyCompare(angle, m_fromAngle)) {
        return;
    }

    m_toAngle = angle;
    m_material->setToAngle(degToRad(angle));
    updateSegments();
}

void PieChartNode::setSmoothEnds(bool smooth)
{
    if (smooth == m_smoothEnds) {
        return;
    }

    m_smoothEnds = smooth;
    m_material->setSmoothEnds(smooth);
    markDirty(QSGNode::DirtyMaterial);
}

void PieChartNode::updateSegments()
{
    if (m_sections.isEmpty() || m_sections.size() != m_colors.size()) {
        return;
    }

    qreal startAngle = degToRad(m_fromAngle);
    qreal totalAngle = degToRad(m_toAngle - m_fromAngle);

    QList<QVector2D> segments;
    QList<QVector4D> colors;

    for (int i = 0; i < m_sections.size(); ++i) {
        QVector2D segment{float(startAngle), float(startAngle + m_sections.at(i) * totalAngle)};
        segments << segment;
        startAngle = segment.y();
        colors << colorToVec4(m_colors.at(i));
    }

    if (m_sections.size() == 1 && qFuzzyCompare(m_sections.at(0), 0.0)) {
        segments.clear();
    }

    m_material->setSegments(segments);
    m_material->setColors(colors);

    markDirty(QSGNode::DirtyMaterial);
}
