/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "GridLines.h"

#include "LineGridNode.h"
#include "XYChart.h"

LinePropertiesGroup::LinePropertiesGroup(GridLines *parent)
    : QObject(parent)
{
    m_parent = parent;
}

bool LinePropertiesGroup::visible() const
{
    return m_visible;
}

void LinePropertiesGroup::setVisible(bool newVisible)
{
    if (newVisible == m_visible) {
        return;
    }

    m_visible = newVisible;
    Q_EMIT propertiesChanged();
}

QColor LinePropertiesGroup::color() const
{
    return m_color;
}

void LinePropertiesGroup::setColor(const QColor &newColor)
{
    if (newColor == m_color) {
        return;
    }

    m_color = newColor;
    Q_EMIT propertiesChanged();
}

float LinePropertiesGroup::lineWidth() const
{
    return m_lineWidth;
}

void LinePropertiesGroup::setLineWidth(float newLineWidth)
{
    if (newLineWidth == m_lineWidth) {
        return;
    }

    m_lineWidth = newLineWidth;
    Q_EMIT propertiesChanged();
}

int LinePropertiesGroup::frequency() const
{
    return m_frequency;
}

void LinePropertiesGroup::setFrequency(int newFrequency)
{
    if (newFrequency == m_frequency) {
        return;
    }

    m_frequency = newFrequency;
    Q_EMIT propertiesChanged();
}

int LinePropertiesGroup::count() const
{
    return m_count;
}

void LinePropertiesGroup::setCount(int newCount)
{
    if (newCount == m_count) {
        return;
    }

    m_count = newCount;
    Q_EMIT propertiesChanged();
}

GridLines::GridLines(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(QQuickItem::ItemHasContents);

    m_major = std::make_unique<LinePropertiesGroup>(this);
    connect(m_major.get(), &LinePropertiesGroup::propertiesChanged, this, &GridLines::update);
    m_minor = std::make_unique<LinePropertiesGroup>(this);
    connect(m_minor.get(), &LinePropertiesGroup::propertiesChanged, this, &GridLines::update);
}

GridLines::Direction GridLines::direction() const
{
    return m_direction;
}

void GridLines::setDirection(GridLines::Direction newDirection)
{
    if (newDirection == m_direction) {
        return;
    }

    m_direction = newDirection;
    update();
    Q_EMIT directionChanged();
}

XYChart *GridLines::chart() const
{
    return m_chart;
}

void GridLines::setChart(XYChart *newChart)
{
    if (newChart == m_chart) {
        return;
    }

    if (m_chart) {
        disconnect(m_chart, &XYChart::computedRangeChanged, this, &GridLines::update);
    }

    m_chart = newChart;

    if (m_chart) {
        connect(m_chart, &XYChart::computedRangeChanged, this, &GridLines::update);
    }

    update();
    Q_EMIT chartChanged();
}

float GridLines::spacing() const
{
    return m_spacing;
}

void GridLines::setSpacing(float newSpacing)
{
    if (newSpacing == m_spacing || m_chart != nullptr) {
        return;
    }

    m_spacing = newSpacing;
    update();
    Q_EMIT spacingChanged();
}

LinePropertiesGroup *GridLines::majorGroup() const
{
    return m_major.get();
}

LinePropertiesGroup *GridLines::minorGroup() const
{
    return m_minor.get();
}

QSGNode *GridLines::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *)
{
    if (!node) {
        node = new QSGNode{};
        node->appendChildNode(new LineGridNode{});
        node->appendChildNode(new LineGridNode{});
    }

    if (m_chart) {
        if (m_direction == Direction::Horizontal) {
            m_spacing = width() / (m_chart->computedRange().distanceX - 1);
        } else {
            m_spacing = height() / (m_chart->computedRange().distanceY);
        }
    }

    updateLines(static_cast<LineGridNode *>(node->childAtIndex(0)), m_minor.get());
    updateLines(static_cast<LineGridNode *>(node->childAtIndex(1)), m_major.get());

    return node;
}

void GridLines::updateLines(LineGridNode *node, LinePropertiesGroup *properties)
{
    node->setVisible(properties->visible());
    node->setRect(boundingRect());
    node->setVertical(m_direction == Direction::Vertical);
    node->setColor(properties->color());
    node->setLineWidth(properties->lineWidth());
    if (properties->count() > 0) {
        node->setSpacing(m_direction == Direction::Horizontal ? width() / (properties->count() + 1) : height() / (properties->count() + 1));
    } else {
        node->setSpacing(m_spacing * properties->frequency());
    }
    node->update();
}

#include "moc_GridLines.cpp"
