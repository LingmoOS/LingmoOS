/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "PieChart.h"

#include <QAbstractItemModel>
#include <QDebug>

#include "RangeGroup.h"
#include "datasource/ChartDataSource.h"
#include "scenegraph/PieChartNode.h"

PieChart::PieChart(QQuickItem *parent)
    : Chart(parent)
{
    setIndexingMode(Chart::IndexSourceValues);
    m_range = std::make_unique<RangeGroup>();
    connect(m_range.get(), &RangeGroup::rangeChanged, this, &PieChart::onDataChanged);
}

RangeGroup *PieChart::range() const
{
    return m_range.get();
}

bool PieChart::filled() const
{
    return m_filled;
}

void PieChart::setFilled(bool newFilled)
{
    if (newFilled == m_filled) {
        return;
    }

    m_filled = newFilled;
    update();
    Q_EMIT filledChanged();
}

qreal PieChart::thickness() const
{
    return m_thickness;
}

void PieChart::setThickness(qreal newThickness)
{
    if (newThickness == m_thickness) {
        return;
    }

    m_thickness = newThickness;
    update();
    Q_EMIT thicknessChanged();
}

qreal PieChart::spacing() const
{
    return m_spacing;
}

void PieChart::setSpacing(qreal newSpacing)
{
    if (newSpacing == m_spacing) {
        return;
    }

    m_spacing = newSpacing;
    update();
    Q_EMIT spacingChanged();
}

QColor PieChart::backgroundColor() const
{
    return m_backgroundColor;
}

void PieChart::setBackgroundColor(const QColor &color)
{
    if (color == m_backgroundColor) {
        return;
    }
    m_backgroundColor = color;
    update();
    Q_EMIT backgroundColorChanged();
}

qreal PieChart::fromAngle() const
{
    return m_fromAngle;
}

void PieChart::setFromAngle(qreal newFromAngle)
{
    if (qFuzzyCompare(newFromAngle, m_fromAngle)) {
        return;
    }

    m_fromAngle = newFromAngle;
    update();
    Q_EMIT fromAngleChanged();
}

qreal PieChart::toAngle() const
{
    return m_toAngle;
}

void PieChart::setToAngle(qreal newToAngle)
{
    if (qFuzzyCompare(newToAngle, m_toAngle)) {
        return;
    }

    m_toAngle = newToAngle;
    update();
    Q_EMIT toAngleChanged();
}

bool PieChart::smoothEnds() const
{
    return m_smoothEnds;
}

void PieChart::setSmoothEnds(bool newSmoothEnds)
{
    if (newSmoothEnds == m_smoothEnds) {
        return;
    }

    m_smoothEnds = newSmoothEnds;
    update();
    Q_EMIT smoothEndsChanged();
}

QSGNode *PieChart::updatePaintNode(QSGNode *node, UpdatePaintNodeData *data)
{
    Q_UNUSED(data);
    if (!node) {
        node = new QSGNode{};
    }

    auto sourceCount = valueSources().size();

    if (m_sections.count() < sourceCount) {
        return node;
    }

    auto minDimension = std::min(width(), height());

    float outerRadius = minDimension;
    for (int i = 0; i < sourceCount; ++i) {
        float innerRadius = i == sourceCount - 1 && m_filled ? 0.0 : outerRadius - m_thickness * 2.0;

        if (node->childCount() <= i) {
            node->appendChildNode(new PieChartNode{});
        }

        auto pieNode = static_cast<PieChartNode *>(node->childAtIndex(i));
        pieNode->setRect(boundingRect());
        pieNode->setInnerRadius(innerRadius);
        pieNode->setOuterRadius(outerRadius);
        pieNode->setSections(m_sections.at(i));
        pieNode->setBackgroundColor(m_backgroundColor);
        pieNode->setColors(m_colors.at(i));
        pieNode->setFromAngle(m_fromAngle);
        pieNode->setToAngle(m_toAngle);
        pieNode->setSmoothEnds(m_smoothEnds);

        outerRadius = innerRadius - m_spacing * 2.0;
    }

    while (node->childCount() > sourceCount) {
        auto lastNode = node->childAtIndex(node->childCount() - 1);
        node->removeChildNode(lastNode);
        delete lastNode;
    }

    return node;
}

void PieChart::onDataChanged()
{
    m_sections.clear();
    m_colors.clear();

    const auto sources = valueSources();
    const auto colors = colorSource();

    if (!colors || sources.isEmpty() || !m_range->isValid()) {
        return;
    }

    auto maximum = [](ChartDataSource *source) {
        qreal result = 0.0;
        for (int i = 0; i < source->itemCount(); ++i) {
            result += source->item(i).toDouble();
        }
        return std::max(result, source->maximum().toDouble());
    };

    auto indexMode = indexingMode();
    auto colorIndex = 0;
    const auto highlightIndex = highlight();
    auto calculateZeroRange = [](ChartDataSource *) {
        return 0.0;
    };
    auto range = m_range->calculateRange(valueSources(), calculateZeroRange, maximum);

    for (auto source : sources) {
        qreal threshold = range.start;
        qreal total = 0.0;

        QList<qreal> sections;
        QList<QColor> sectionColors;

        for (int i = 0; i < source->itemCount(); ++i) {
            auto value = source->item(i).toReal();
            auto limited = value - threshold;
            if (limited > 0.0) {
                if (total + limited >= range.end) {
                    limited = range.end - total;
                }

                sections << limited;
                total += limited;

                auto color = colors->item(colorIndex).value<QColor>();

                if (highlightIndex >= 0 && highlightIndex != colorIndex) {
                    color = desaturate(color);
                }

                sectionColors << color;
            }
            threshold = std::max(0.0, threshold - value);

            if (indexMode != IndexEachSource) {
                colorIndex++;
            }
        }

        if (qFuzzyCompare(total, 0.0)) {
            m_sections << QList<qreal>{0.0};
            m_colors << QList<QColor>{colors->item(colorIndex).value<QColor>()};
        }

        for (auto &value : sections) {
            value = value / range.distance;
        }

        m_sections << sections;
        m_colors << sectionColors;

        if (indexMode == IndexEachSource) {
            colorIndex++;
        } else if (indexMode == IndexSourceValues) {
            colorIndex = 0;
        }
    }

    update();
}

#include "moc_PieChart.cpp"
