/*
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "BarChart.h"

#include <QDebug>
#include <QSGNode>

#include "RangeGroup.h"
#include "datasource/ChartDataSource.h"
#include "scenegraph/BarChartNode.h"

BarChart::BarChart(QQuickItem *parent)
    : XYChart(parent)
{
}

qreal BarChart::spacing() const
{
    return m_spacing;
}

void BarChart::setSpacing(qreal newSpacing)
{
    if (newSpacing == m_spacing) {
        return;
    }

    m_spacing = newSpacing;
    update();
    Q_EMIT spacingChanged();
}

qreal BarChart::barWidth() const
{
    return m_barWidth;
}

void BarChart::setBarWidth(qreal newBarWidth)
{
    if (newBarWidth == m_barWidth) {
        return;
    }

    m_barWidth = newBarWidth;
    update();
    Q_EMIT barWidthChanged();
}

qreal BarChart::radius() const
{
    return m_radius;
}

void BarChart::setRadius(qreal newRadius)
{
    if (newRadius == m_radius) {
        return;
    }

    m_radius = newRadius;
    update();
    Q_EMIT radiusChanged();
}

BarChart::Orientation BarChart::orientation() const
{
    return m_orientation;
}

void BarChart::setOrientation(BarChart::Orientation newOrientation)
{
    if (newOrientation == m_orientation) {
        return;
    }

    m_orientation = newOrientation;
    m_orientationChanged = true;
    update();
    Q_EMIT orientationChanged();
}

QColor BarChart::backgroundColor() const
{
    return m_backgroundColor;
}

void BarChart::setBackgroundColor(const QColor &newBackgroundColor)
{
    if (newBackgroundColor == m_backgroundColor) {
        return;
    }

    m_backgroundColor = newBackgroundColor;
    update();
    Q_EMIT backgroundColorChanged();
}

QSGNode *BarChart::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *)
{
    BarChartNode *barNode = nullptr;

    if (m_orientationChanged) {
        delete node;
        node = nullptr;
        m_orientationChanged = false;
    }

    if (!node) {
        barNode = new BarChartNode{};
        if (m_orientation == VerticalOrientation) {
            node = barNode;
        } else {
            auto transformNode = new QSGTransformNode{};
            transformNode->appendChildNode(barNode);
            QMatrix4x4 matrix;
            matrix.translate(width(), 0.0);
            matrix.rotate(90.0, 0.0, 0.0, 1.0);
            transformNode->setMatrix(matrix);
            node = transformNode;
        }
    } else {
        if (m_orientation == VerticalOrientation) {
            barNode = static_cast<BarChartNode *>(node);
        } else {
            barNode = static_cast<BarChartNode *>(node->childAtIndex(0));
        }
    }

    if (m_orientation == VerticalOrientation) {
        barNode->setRect(boundingRect());
    } else {
        QMatrix4x4 matrix;
        matrix.translate(width(), 0.0);
        matrix.rotate(90.0, 0.0, 0.0, 1.0);
        static_cast<QSGTransformNode *>(node)->setMatrix(matrix);
        barNode->setRect(QRectF{boundingRect().topLeft(), QSizeF{height(), width()}});
    }
    barNode->setBars(calculateBars());
    barNode->setRadius(m_radius);
    barNode->setBackgroundColor(m_backgroundColor);

    barNode->update();

    return node;
}

void BarChart::onDataChanged()
{
    if (valueSources().size() == 0 || !colorSource()) {
        return;
    }

    m_barDataItems.clear();

    updateComputedRange();

    const auto range = computedRange();
    const auto sources = valueSources();
    auto colors = colorSource();
    auto indexMode = indexingMode();
    auto colorIndex = 0;

    m_barDataItems.fill(QList<BarData>{}, range.distanceX);

    const auto highlightIndex = highlight();

    auto generator = [&, this, i = range.startX]() mutable -> QList<BarData> {
        QList<BarData> colorInfos;

        for (int j = 0; j < sources.count(); ++j) {
            auto value = (sources.at(j)->item(i).toReal() - range.startY) / range.distanceY;
            auto color = colors->item(colorIndex).value<QColor>();

            if (highlightIndex >= 0 && highlightIndex != colorIndex) {
                color = desaturate(color);
            }

            colorInfos << BarData{value, color};

            if (indexMode != Chart::IndexSourceValues) {
                colorIndex++;
            }
        }

        if (stacked()) {
            auto previous = 0.0;
            for (auto &[colorVal, _] : colorInfos) {
                colorVal += previous;
                previous = colorVal;
            }
        }

        if (indexMode == Chart::IndexSourceValues) {
            colorIndex++;
        } else if (indexMode == Chart::IndexEachSource) {
            colorIndex = 0;
        }

        i++;
        return colorInfos;
    };

    if (direction() == Direction::ZeroAtStart) {
        std::generate_n(m_barDataItems.begin(), range.distanceX, generator);
    } else {
        std::generate_n(m_barDataItems.rbegin(), range.distanceX, generator);
    }

    update();
}

QList<Bar> BarChart::calculateBars()
{
    QList<Bar> result;

    // TODO: Find some way to clean this up and simplify it, since this is pretty ugly.

    auto targetWidth = m_orientation == VerticalOrientation ? width() : height();

    float w = m_barWidth;
    if (w < 0.0) {
        const auto totalItemCount = stacked() ? m_barDataItems.size() : m_barDataItems.size() * valueSources().count();

        w = targetWidth / totalItemCount - m_spacing;

        auto x = float(m_spacing / 2);
        const auto itemSpacing = w + m_spacing;

        for (const auto &items : std::as_const(m_barDataItems)) {
            result.reserve(result.size() + items.size());
            if (stacked()) {
                std::transform(items.crbegin(), items.crend(), std::back_inserter(result), [x, w](const BarData &entry) {
                    return Bar{x, w, float(entry.value), entry.color};
                });
                x += itemSpacing;
            } else {
                std::transform(items.cbegin(), items.cend(), std::back_inserter(result), [&x, itemSpacing, w](const BarData &entry) {
                    Bar bar{x, w, float(entry.value), entry.color};
                    x += itemSpacing;
                    return bar;
                });
            }
        }
    } else {
        const auto itemSpacing = targetWidth / m_barDataItems.size();
        if (stacked()) {
            auto x = float(itemSpacing / 2 - m_barWidth / 2);

            for (const auto &items : std::as_const(m_barDataItems)) {
                result.reserve(result.size() + items.size());
                std::transform(items.crbegin(), items.crend(), std::back_inserter(result), [x, w](const BarData &entry) {
                    return Bar{x, w, float(entry.value), entry.color};
                });

                x += itemSpacing;
            }
        } else {
            const auto totalWidth = m_barWidth * valueSources().count() + m_spacing * (valueSources().count() - 1);

            auto x = float(itemSpacing / 2 - totalWidth / 2);

            for (const auto &items : std::as_const(m_barDataItems)) {
                result.reserve(result.size() + items.size());
                for (int i = 0; i < items.count(); ++i) {
                    auto entry = items.at(i);
                    result << Bar{float(x + i * (m_barWidth + m_spacing)), w, float(entry.value), entry.color};
                }
                x += itemSpacing;
            }
        }
    }

    return result;
}

#include "moc_BarChart.cpp"
