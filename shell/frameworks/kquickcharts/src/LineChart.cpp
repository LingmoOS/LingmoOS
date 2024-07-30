/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "LineChart.h"

#include <cmath>

#include <QPainter>
#include <QPainterPath>
#include <QQuickWindow>

#include "RangeGroup.h"
#include "datasource/ChartDataSource.h"
#include "scenegraph/LineChartNode.h"

static const float PixelsPerStep = 2.0;


QList<QVector2D> interpolatePoints(const QList<QVector2D> &points, float height);
QList<float> calculateTangents(const QList<QVector2D> &points, float height);
QVector2D cubicHermite(const QVector2D &first, const QVector2D &second, float step, float mFirst, float mSecond);

QColor colorWithAlpha(const QColor &color, qreal opacity)
{
    auto result = color;
    result.setRedF(result.redF() * opacity);
    result.setGreenF(result.greenF() * opacity);
    result.setBlueF(result.blueF() * opacity);
    result.setAlphaF(opacity);
    return result;
}

LineChartAttached::LineChartAttached(QObject *parent)
    : QObject(parent)
{
}

QVariant LineChartAttached::value() const
{
    return m_value;
}

void LineChartAttached::setValue(const QVariant &value)
{
    if (value == m_value) {
        return;
    }

    m_value = value;
    Q_EMIT valueChanged();
}

QColor LineChartAttached::color() const
{
    return m_color;
}

void LineChartAttached::setColor(const QColor &color)
{
    if (color == m_color) {
        return;
    }

    m_color = color;
    Q_EMIT colorChanged();
}

QString LineChartAttached::name() const
{
    return m_name;
}

void LineChartAttached::setName(const QString &newName)
{
    if (newName == m_name) {
        return;
    }

    m_name = newName;
    Q_EMIT nameChanged();
}

QString LineChartAttached::shortName() const
{
    if (m_shortName.isEmpty()) {
        return m_name;
    } else {
        return m_shortName;
    }
}

void LineChartAttached::setShortName(const QString &newShortName)
{
    if (newShortName == m_shortName) {
        return;
    }

    m_shortName = newShortName;
    Q_EMIT shortNameChanged();
}

LineChart::LineChart(QQuickItem *parent)
    : XYChart(parent)
{
}

bool LineChart::interpolate() const
{
    return m_interpolate;
}

qreal LineChart::lineWidth() const
{
    return m_lineWidth;
}

qreal LineChart::fillOpacity() const
{
    return m_fillOpacity;
}

void LineChart::setInterpolate(bool newInterpolate)
{
    if (newInterpolate == m_interpolate) {
        return;
    }

    m_interpolate = newInterpolate;
    polish();
    Q_EMIT interpolateChanged();
}

void LineChart::setLineWidth(qreal width)
{
    if (qFuzzyCompare(m_lineWidth, width)) {
        return;
    }

    m_lineWidth = width;
    update();
    Q_EMIT lineWidthChanged();
}

void LineChart::setFillOpacity(qreal opacity)
{
    if (qFuzzyCompare(m_fillOpacity, opacity)) {
        return;
    }

    m_fillOpacity = opacity;
    update();
    Q_EMIT fillOpacityChanged();
}

ChartDataSource *LineChart::fillColorSource() const
{
    return m_fillColorSource;
}

void LineChart::setFillColorSource(ChartDataSource *newFillColorSource)
{
    if (newFillColorSource == m_fillColorSource) {
        return;
    }

    m_fillColorSource = newFillColorSource;
    update();
    Q_EMIT fillColorSourceChanged();
}

QQmlComponent *LineChart::pointDelegate() const
{
    return m_pointDelegate;
}

void LineChart::setPointDelegate(QQmlComponent *newPointDelegate)
{
    if (newPointDelegate == m_pointDelegate) {
        return;
    }

    m_pointDelegate = newPointDelegate;
    for (auto entry : std::as_const(m_pointDelegates)) {
        qDeleteAll(entry);
    }
    m_pointDelegates.clear();
    polish();
    Q_EMIT pointDelegateChanged();
}

void LineChart::updatePolish()
{
    if (m_rangeInvalid) {
        updateComputedRange();
        m_rangeInvalid = false;
    }

    QList<QVector2D> previousValues;

    const auto range = computedRange();
    const auto sources = valueSources();
    for (int i = 0; i < sources.size(); ++i) {
        auto valueSource = sources.at(i);

        float stepSize = width() / (range.distanceX - 1);
        QList<QVector2D> values(range.distanceX);
        auto generator = [&, i = range.startX]() mutable -> QVector2D {
            float value = 0;
            if (range.distanceY != 0) {
                value = (valueSource->item(i).toFloat() - range.startY) / range.distanceY;
            }

            auto result = QVector2D{direction() == Direction::ZeroAtStart ? i * stepSize : float(boundingRect().right()) - i * stepSize, value};
            i++;
            return result;
        };

        if (direction() == Direction::ZeroAtStart) {
            std::generate_n(values.begin(), range.distanceX, generator);
        } else {
            std::generate_n(values.rbegin(), range.distanceX, generator);
        }

        if (stacked() && !previousValues.isEmpty()) {
            if (values.size() != previousValues.size()) {
                qWarning() << "Value source" << valueSource->objectName()
                           << "has a different number of elements from the previous source. Ignoring stacking for this source.";
            } else {
                std::for_each(values.begin(), values.end(), [previousValues, i = 0](QVector2D &point) mutable {
                    point.setY(point.y() + previousValues.at(i++).y());
                });
            }
        }
        previousValues = values;

        if (m_pointDelegate) {
            auto &delegates = m_pointDelegates[valueSource];
            if (delegates.size() != values.size()) {
                qDeleteAll(delegates);
                createPointDelegates(values, i);
            } else {
                for (int item = 0; item < values.size(); ++item) {
                    auto delegate = delegates.at(item);
                    updatePointDelegate(delegate, values.at(item), valueSource->item(item), i);
                }
            }
        }

        if (m_interpolate) {
            m_values[valueSource] = interpolatePoints(values, height());
        } else {
            m_values[valueSource] = values;
        }
    }

    const auto pointKeys = m_pointDelegates.keys();
    for (auto key : pointKeys) {
        if (!sources.contains(key)) {
            qDeleteAll(m_pointDelegates[key]);
            m_pointDelegates.remove(key);
        }
    }

    update();
}

QSGNode *LineChart::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *data)
{
    Q_UNUSED(data);

    if (!node) {
        node = new QSGNode();
    }

    const auto highlightIndex = highlight();
    const auto sources = valueSources();
    for (int i = 0; i < sources.size(); ++i) {
        int childIndex = sources.size() - 1 - i;
        while (childIndex >= node->childCount()) {
            node->appendChildNode(new LineChartNode{});
        }
        auto lineNode = static_cast<LineChartNode *>(node->childAtIndex(childIndex));
        auto color = colorSource() ? colorSource()->item(i).value<QColor>() : Qt::black;
        auto fillColor = m_fillColorSource ? m_fillColorSource->item(i).value<QColor>() : colorWithAlpha(color, m_fillOpacity);
        auto lineWidth = i == highlightIndex ? std::max(m_lineWidth, 3.0) : m_lineWidth;

        if (highlightIndex >= 0 && i != highlightIndex) {
            color = desaturate(color);
            fillColor = desaturate(fillColor);
        }

        updateLineNode(lineNode, sources.at(i), color, fillColor, lineWidth);
    }

    while (node->childCount() > sources.size()) {
        // removeChildNode unfortunately does not take care of deletion so we
        // need to handle this manually.
        auto lastNode = node->childAtIndex(node->childCount() - 1);
        node->removeChildNode(lastNode);
        delete lastNode;
    }

    if (highlightIndex >= 0) {
        // Move highlighted node to the end to ensure we always show the
        // highlighted chart on top. This is done after the above removal to
        // ensure we don't suddenly remove the highlighted node.
        auto highlightNode = node->childAtIndex(node->childCount() - 1 - highlightIndex);
        node->removeChildNode(highlightNode);
        node->appendChildNode(highlightNode);
    }

    return node;
}

void LineChart::onDataChanged()
{
    m_rangeInvalid = true;
    polish();
}

void LineChart::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    XYChart::geometryChange(newGeometry, oldGeometry);
    if (newGeometry != oldGeometry) {
        polish();
    }
}

void LineChart::updateLineNode(LineChartNode *node, ChartDataSource *valueSource, const QColor &lineColor, const QColor &fillColor, qreal lineWidth)
{
    if (window()) {
        node->setRect(boundingRect(), window()->devicePixelRatio());
    } else {
        node->setRect(boundingRect(), 1.0);
    }
    node->setLineColor(lineColor);
    node->setFillColor(fillColor);
    node->setLineWidth(lineWidth);

    auto values = m_values.value(valueSource);
    node->setValues(values);

    node->updatePoints();
}

void LineChart::createPointDelegates(const QList<QVector2D> &values, int sourceIndex)
{
    auto valueSource = valueSources().at(sourceIndex);

    QList<QQuickItem *> delegates;
    for (int i = 0; i < values.size(); ++i) {
        auto delegate = qobject_cast<QQuickItem *>(m_pointDelegate->beginCreate(qmlContext(m_pointDelegate)));
        if (!delegate) {
            qWarning() << "Delegate creation for point" << i << "of value source" << valueSource->objectName()
                       << "failed, make sure pointDelegate is a QQuickItem";
            delegate = new QQuickItem(this);
        }

        delegate->setParent(this);
        delegate->setParentItem(this);
        updatePointDelegate(delegate, values.at(i), valueSource->item(i), sourceIndex);

        m_pointDelegate->completeCreate();

        delegates.append(delegate);
    }

    m_pointDelegates.insert(valueSource, delegates);
}

void LineChart::updatePointDelegate(QQuickItem *delegate, const QVector2D &position, const QVariant &value, int sourceIndex)
{
    auto pos = QPointF{position.x() - delegate->width() / 2, (1.0 - position.y()) * height() - delegate->height() / 2};
    delegate->setPosition(pos);

    auto color = colorSource() ? colorSource()->item(sourceIndex).value<QColor>() : QColor();
    auto highlightIndex = highlight();
    if (highlightIndex >= 0) {
        if (sourceIndex == highlightIndex) {
            delegate->setZ(1.0);
        } else {
            color = desaturate(color);
        }
    } else {
        delegate->setZ(0.0);
    }

    auto attached = static_cast<LineChartAttached *>(qmlAttachedPropertiesObject<LineChart>(delegate, true));
    attached->setValue(value);
    attached->setColor(color);
    attached->setName(nameSource() ? nameSource()->item(sourceIndex).toString() : QString{});
    attached->setShortName(shortNameSource() ? shortNameSource()->item(sourceIndex).toString() : QString{});
}

// Smoothly interpolate between points, using monotonic cubic interpolation.
QList<QVector2D> interpolatePoints(const QList<QVector2D> &points, float height)
{
    if (points.size() < 2) {
        return points;
    }

    auto tangents = calculateTangents(points, height);

    QList<QVector2D> result;

    auto current = QVector2D{0.0, points.first().y() * height};
    result.append(QVector2D{0.0, points.first().y()});

    for (int i = 0; i < points.size() - 1; ++i) {
        auto next = QVector2D{points.at(i + 1).x(), points.at(i + 1).y() * height};

        auto currentTangent = tangents.at(i);
        auto nextTangent = tangents.at(i + 1);

        auto stepCount = int(std::max(1.0f, (next.x() - current.x()) / PixelsPerStep));
        auto stepSize = (next.x() - current.x()) / stepCount;

        if (stepCount == 1 || qFuzzyIsNull(next.y() - current.y())) {
            result.append(QVector2D{next.x(), next.y() / height});
            current = next;
            continue;
        }

        for (auto delta = current.x(); delta < next.x(); delta += stepSize) {
            auto interpolated = cubicHermite(current, next, delta, currentTangent, nextTangent);
            interpolated.setY(interpolated.y() / height);
            result.append(interpolated);
        }

        current = next;
    }

    current.setY(current.y() / height);
    result.append(current);

    return result;
}

// This calculates the tangents for monotonic cubic spline interpolation.
// See https://en.wikipedia.org/wiki/Monotone_cubic_interpolation for details.
QList<float> calculateTangents(const QList<QVector2D> &points, float height)
{
    QList<float> secantSlopes;
    secantSlopes.reserve(points.size());

    QList<float> tangents;
    tangents.reserve(points.size());

    float previousSlope = 0.0;
    float slope = 0.0;

    for (int i = 0; i < points.size() - 1; ++i) {
        auto current = points.at(i);
        auto next = points.at(i + 1);

        previousSlope = slope;
        slope = (next.y() * height - current.y() * height) / (next.x() - current.x());

        secantSlopes.append(slope);

        if (i == 0) {
            tangents.append(slope);
        } else if (previousSlope * slope < 0.0) {
            tangents.append(0.0);
        } else {
            tangents.append((previousSlope + slope) / 2.0);
        }
    }
    tangents.append(secantSlopes.last());

    for (int i = 0; i < points.size() - 1; ++i) {
        auto slope = secantSlopes.at(i);

        if (qFuzzyIsNull(slope)) {
            tangents[i] = 0.0;
            tangents[i + 1] = 0.0;
            continue;
        }

        auto alpha = tangents.at(i) / slope;
        auto beta = tangents.at(i + 1) / slope;

        if (alpha < 0.0) {
            tangents[i] = 0.0;
        }

        if (beta < 0.0) {
            tangents[i + 1] = 0.0;
        }

        auto length = alpha * alpha + beta * beta;
        if (length > 9) {
            auto tau = 3.0 / sqrt(length);
            tangents[i] = tau * alpha * slope;
            tangents[i + 1] = tau * beta * slope;
        }
    }

    return tangents;
}

// Cubic Hermite Interpolation between two points
// Given two points, an X value between those two points and two tangents, this
// will perform cubic hermite interpolation between the two points.
// See https://en.wikipedia.org/wiki/Cubic_Hermite_spline for details as well as
// the above mentioned article on monotonic interpolation.
QVector2D cubicHermite(const QVector2D &first, const QVector2D &second, float step, float mFirst, float mSecond)
{
    const auto delta = second.x() - first.x();
    const auto t = (step - first.x()) / delta;

    // Hermite basis values
    // h₀₀(t) = 2t³ - 3t² + 1
    const auto h00 = 2.0f * std::pow(t, 3.0f) - 3.0f * std::pow(t, 2.0f) + 1.0f;
    // h₁₀(t) = t³ - 2t² + t
    const auto h10 = std::pow(t, 3.0f) - 2.0f * std::pow(t, 2.0f) + t;
    // h₀₁(t) = -2t³ + 3t²
    const auto h01 = -2.0f * std::pow(t, 3.0f) + 3.0f * std::pow(t, 2.0f);
    // h₁₁(t) = t³ - t²
    const auto h11 = std::pow(t, 3.0f) - std::pow(t, 2.0f);

    auto result = QVector2D{step, first.y() * h00 + delta * mFirst * h10 + second.y() * h01 + delta * mSecond * h11};
    return result;
}

#include "moc_LineChart.cpp"
