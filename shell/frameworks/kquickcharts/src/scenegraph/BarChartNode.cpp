/*
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "BarChartNode.h"

#include <QColor>
#include <QDebug>

#include "BarChartMaterial.h"

struct BarVertex {
    float x;
    float y;

    float u;
    float v;

    float r;
    float g;
    float b;
    float a;

    float value;

    void set(const QPointF &position, const QVector2D &uv, const QColor &color, float newValue)
    {
        x = position.x();
        y = position.y();
        u = uv.x();
        v = uv.y();
        r = color.redF();
        g = color.greenF();
        b = color.blueF();
        a = color.alphaF();
        value = newValue;
    }
};

/* clang-format off */
QSGGeometry::Attribute BarAttributes[] = {
    QSGGeometry::Attribute::create(0, 2, QSGGeometry::FloatType, true),
    QSGGeometry::Attribute::create(1, 2, QSGGeometry::FloatType, false),
    QSGGeometry::Attribute::create(2, 4, QSGGeometry::FloatType, false),
    QSGGeometry::Attribute::create(3, 1, QSGGeometry::FloatType, false)
};
/* clang-format on */

QSGGeometry::AttributeSet BarAttributeSet = {4, sizeof(BarVertex), BarAttributes};

void updateBarGeometry(QSGGeometry *geometry, const QRectF &rect, const QColor &color, float value)
{
    auto vertices = static_cast<BarVertex *>(geometry->vertexData());
    vertices[0].set(rect.topLeft(), {0.0, 0.0}, color, value);
    vertices[1].set(rect.bottomLeft(), {0.0, 1.0}, color, value);
    vertices[2].set(rect.topRight(), {1.0, 0.0}, color, value);
    vertices[3].set(rect.bottomRight(), {1.0, 1.0}, color, value);
    geometry->markVertexDataDirty();
}

class BarNode : public QSGGeometryNode
{
public:
    BarNode(const QRectF &r)
    {
        geometry = new QSGGeometry(BarAttributeSet, 4);
        geometry->setVertexDataPattern(QSGGeometry::DynamicPattern);
        updateBarGeometry(geometry, r, Qt::transparent, 0.0);
        setGeometry(geometry);

        rect = r;

        material = new BarChartMaterial{};
        setMaterial(material);

        setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial);
    }

    void update()
    {
        auto minSize = std::min(rect.width(), rect.height());
        auto aspect = rect.height() / minSize;
        updateBarGeometry(geometry, rect, color, value * aspect);

        markDirty(QSGNode::DirtyGeometry);
    }

    QSGGeometry *geometry;
    BarChartMaterial *material;
    QRectF rect;
    QColor color;
    float value;
};

BarChartNode::BarChartNode()
{
}

void BarChartNode::setRect(const QRectF &rect)
{
    m_rect = rect;
}

void BarChartNode::setBars(const QList<Bar> &bars)
{
    m_bars = bars;
}

void BarChartNode::setRadius(qreal radius)
{
    m_radius = radius;
}

void BarChartNode::setBackgroundColor(const QColor &color)
{
    m_backgroundColor = color;
}

void BarChartNode::update()
{
    if (!m_rect.isValid() || m_bars.isEmpty()) {
        return;
    }

    for (auto index = 0; index < m_bars.count(); ++index) {
        auto entry = m_bars.at(index);

        auto rect = QRectF{QPointF{entry.x, m_rect.top()}, QSizeF{entry.width, m_rect.height()}};

        if (childCount() <= index) {
            appendChildNode(new BarNode{rect});
        }

        auto child = static_cast<BarNode *>(childAtIndex(index));

        auto minSize = std::min(rect.width(), rect.height());
        auto aspect = QVector2D{float(rect.width() / minSize), float(rect.height() / minSize)};

        if (aspect != child->material->aspect) {
            child->material->aspect = aspect;
            child->markDirty(QSGNode::DirtyMaterial);
        }

        float correctedRadius = (std::min(m_radius, entry.width / 2.0) / minSize) * 2.0;
        if (!qFuzzyCompare(correctedRadius, child->material->radius)) {
            child->material->radius = correctedRadius;
            child->markDirty(QSGNode::DirtyMaterial);
        }

        if (m_backgroundColor != child->material->backgroundColor) {
            child->material->backgroundColor = m_backgroundColor;
            child->markDirty(QSGNode::DirtyMaterial);
        }

        if (child->rect != rect || !qFuzzyCompare(child->value, entry.value) || child->color != entry.color) {
            child->rect = rect;
            child->value = entry.value;
            child->color = entry.color;
            child->update();
        }
    }

    while (childCount() > m_bars.count()) {
        auto child = childAtIndex(childCount() - 1);
        removeChildNode(child);
        delete child;
    }
}
