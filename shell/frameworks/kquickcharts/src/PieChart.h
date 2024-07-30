/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef PIECHART_H
#define PIECHART_H

#include <memory>

#include "Chart.h"
#include "RangeGroup.h"

/**
 * An item to render a pie chart.
 *
 * This item will render a Pie chart based on the [valueSources] supplied to it.
 * By default it will set [indexingMode] to [IndexSourceValues], meaning that it
 * will use the individual values of the valueSources to render sections of the
 * chart.
 *
 * [valueSources]: \ref Chart::valueSources
 * [indexingMode]: \ref Chart::indexingMode
 * [IndexSourceValues]: \ref Chart::IndexingMode
 *
 * ### Usage example
 *
 * \snippet snippets/piechart.qml example
 *
 * \image html piechart.png "The resulting pie chart."
 *
 * ### Multiple Value Sources
 *
 * Multiple valueSources are rendered as consecutive pie charts in the same
 * item. The first valueSource will become the outermost circle and consecutive
 * valueSources will be rendered as continuously smaller circles. When rendering
 * multiple value sources, [filled] will only affect the last value source. All
 * other value sources will be rendered according to [thickness], with [spacing]
 * amount of space between them.
 *
 * [filled]: \ref PieChart::filled
 * [thickness]: \ref PieChart::thickness
 * [spacing]: \ref PieChart::spacing
 *
 */
class QUICKCHARTS_EXPORT PieChart : public Chart
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit PieChart(QQuickItem *parent = nullptr);

    /**
     * The range of values to display in this PieChart.
     *
     * When set to "automatic", the values will be divided across the entire
     * chart.
     *
     * \sa RangeGroup
     */
    Q_PROPERTY(RangeGroup *range READ range CONSTANT)
    RangeGroup *range() const;
    /**
     * Whether to use a filled pie or not.
     *
     * If true, the last pie rendered will be rendered as a filled circle.
     * The default is false.
     */
    Q_PROPERTY(bool filled READ filled WRITE setFilled NOTIFY filledChanged)
    bool filled() const;
    void setFilled(bool newFilled);
    Q_SIGNAL void filledChanged();
    /**
     * The thickness of an individual pie, in pixels.
     *
     * If filled is set, this is ignored for the last pie. The default is 10px.
     */
    Q_PROPERTY(qreal thickness READ thickness WRITE setThickness NOTIFY thicknessChanged)
    qreal thickness() const;
    void setThickness(qreal newThickness);
    Q_SIGNAL void thicknessChanged();
    /**
     * The amount of spacing between pies when rendering multiple value sources.
     *
     * The default is 0.
     */
    Q_PROPERTY(qreal spacing READ spacing WRITE setSpacing NOTIFY spacingChanged)
    qreal spacing() const;
    void setSpacing(qreal newSpacing);
    Q_SIGNAL void spacingChanged();
    /**
     * Sets a colour to use to fill remaining space on the pie.
     *
     * The default is transparent.
     */
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &color);
    Q_SIGNAL void backgroundColorChanged();
    /**
     * The starting angle of the arc used for the entire pie.
     *
     * When set, instead of rendering a full circle, the pie will be rendered as
     * an arc. The default is 0.
     */
    Q_PROPERTY(qreal fromAngle READ fromAngle WRITE setFromAngle NOTIFY fromAngleChanged)
    qreal fromAngle() const;
    void setFromAngle(qreal newFromAngle);
    Q_SIGNAL void fromAngleChanged();
    /**
     * The end angle of the arc used for the entire pie. When set, instead of
     * rendering a full circle, the pie will be rendered as an arc. The default
     * is 360.
     */
    Q_PROPERTY(qreal toAngle READ toAngle WRITE setToAngle NOTIFY toAngleChanged)
    qreal toAngle() const;
    void setToAngle(qreal newToAngle);
    Q_SIGNAL void toAngleChanged();
    /**
     * Smooth the ends of pie sections.
     *
     * When true, this will try to smooth the ends of sections. This works best
     * when [filled] is false. The default is false.
     *
     * [filled]: \ref PieChart::filled
     */
    Q_PROPERTY(bool smoothEnds READ smoothEnds WRITE setSmoothEnds NOTIFY smoothEndsChanged)
    bool smoothEnds() const;
    void setSmoothEnds(bool newSmoothEnds);
    Q_SIGNAL void smoothEndsChanged();

protected:
    /**
     * Reimplemented from QQuickItem.
     */
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) override;
    /**
     * Reimplemented from Chart.
     */
    void onDataChanged() override;

private:
    std::unique_ptr<RangeGroup> m_range;
    bool m_filled = false;
    qreal m_thickness = 10.0;
    qreal m_spacing = 0.0;
    QColor m_backgroundColor = Qt::transparent;
    qreal m_fromAngle = 0.0;
    qreal m_toAngle = 360.0;
    bool m_smoothEnds = false;

    QList<QList<qreal>> m_sections;
    QList<QList<QColor>> m_colors;
};

#endif // PIECHART_H
