/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "LineGridNode.h"

#include <cmath>

#include <QSGFlatColorMaterial>

LineGridNode::LineGridNode()
{
    m_geometry = new QSGGeometry{QSGGeometry::defaultAttributes_Point2D(), 0};
    m_geometry->setDrawingMode(QSGGeometry::DrawLines);
    m_geometry->setLineWidth(m_lineWidth);
    setGeometry(m_geometry);

    m_material = new QSGFlatColorMaterial{};
    m_material->setColor(QColor(255, 0, 0, 255));
    setMaterial(m_material);

    setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial);
}

LineGridNode::~LineGridNode()
{
}

void LineGridNode::setVisible(bool visible)
{
    if (visible == m_visible) {
        return;
    }

    m_visible = visible;
    markDirty(QSGNode::DirtySubtreeBlocked);
}

void LineGridNode::setVertical(bool vertical)
{
    if (vertical == m_vertical) {
        return;
    }

    m_vertical = vertical;
}

void LineGridNode::setRect(const QRectF &rect)
{
    if (rect == m_rect) {
        return;
    }

    m_rect = rect;
}

void LineGridNode::setColor(const QColor &color)
{
    if (color == m_material->color()) {
        return;
    }

    m_material->setColor(color);
    markDirty(QSGNode::DirtyMaterial);
}

void LineGridNode::setSpacing(float spacing)
{
    if (qFuzzyCompare(spacing, m_spacing)) {
        return;
    }

    m_spacing = spacing;
}

void LineGridNode::setLineWidth(float lineWidth)
{
    if (qFuzzyCompare(lineWidth, m_lineWidth)) {
        return;
    }

    m_lineWidth = lineWidth;
    m_geometry->setLineWidth(lineWidth);
    markDirty(QSGNode::DirtyGeometry);
}

bool LineGridNode::isSubtreeBlocked() const
{
    return !m_visible;
}

void LineGridNode::update()
{
    if (!m_rect.isValid()) {
        return;
    }

    int totalVertices = 0;
    if (!m_vertical) {
        totalVertices = std::floor(m_rect.width() / std::ceil(m_spacing)) * 2 + 4;
    } else {
        totalVertices = std::floor(m_rect.height() / std::ceil(m_spacing)) * 2 + 4;
    }

    if (totalVertices < 4) {
        return;
    }

    if (totalVertices != m_geometry->vertexCount()) {
        m_geometry->allocate(totalVertices, totalVertices);
    }

    auto vertices = m_geometry->vertexDataAsPoint2D();
    auto indices = m_geometry->indexDataAsUShort();

    if (!vertices || !indices) {
        return;
    }

    int index = 0;
    if (m_vertical) {
        line(vertices, indices, index, m_rect.left(), m_rect.top(), m_rect.right(), m_rect.top());

        auto y = m_spacing;
        for (auto i = 0; i < (totalVertices - 4) / 2; ++i) {
            line(vertices, indices, index, m_rect.left(), y, m_rect.right(), y);
            y += m_spacing;
        }

        line(vertices, indices, index, m_rect.left(), m_rect.bottom(), m_rect.right(), m_rect.bottom());
    } else {
        line(vertices, indices, index, m_rect.left(), m_rect.top(), m_rect.left(), m_rect.bottom());

        auto x = m_spacing;
        for (auto i = 0; i < (totalVertices - 4) / 2; ++i) {
            line(vertices, indices, index, x, m_rect.top(), x, m_rect.bottom());
            x += m_spacing;
        }

        line(vertices, indices, index, m_rect.right(), m_rect.top(), m_rect.right(), m_rect.bottom());
    }

    m_geometry->markVertexDataDirty();
    m_geometry->markIndexDataDirty();
    markDirty(QSGNode::DirtyGeometry);
}

void LineGridNode::line(QSGGeometry::Point2D *vertices, quint16 *indices, int &index, qreal fromX, qreal fromY, qreal toX, qreal toY)
{
    indices[index] = index;
    vertices[index++].set(fromX, fromY);
    indices[index] = index;
    vertices[index++].set(toX, toY);
}
