/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef LINECHART_H
#define LINECHART_H

#include <memory>

#include <qqmlregistration.h>

#include "XYChart.h"

class LineChartNode;

/**
 * An attached property that is exposed to point delegates created in line charts.
 *
 * \sa LineChart::pointDelegate
 */
class LineChartAttached : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS

public:
    LineChartAttached(QObject *parent = nullptr);

    /**
     * The value at the current point.
     */
    Q_PROPERTY(QVariant value READ value NOTIFY valueChanged)
    QVariant value() const;
    void setValue(const QVariant &value);
    Q_SIGNAL void valueChanged();

    /**
     * The color at the current point.
     */
    Q_PROPERTY(QColor color READ color NOTIFY colorChanged)
    QColor color() const;
    void setColor(const QColor &color);
    Q_SIGNAL void colorChanged();

    /**
     * The name at the current point.
     */
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    QString name() const;
    void setName(const QString &newName);
    Q_SIGNAL void nameChanged();

    /**
     * The short name at the current point.
     */
    Q_PROPERTY(QString shortName READ shortName NOTIFY shortNameChanged)
    QString shortName() const;
    void setShortName(const QString &newShortName);
    Q_SIGNAL void shortNameChanged();

private:
    QVariant m_value;
    QColor m_color;
    QString m_name;
    QString m_shortName;
};

/**
 * A line chart.
 *
 * ## Usage example
 *
 * \snippet snippets/linechart.qml example
 *
 * \image html linechart.png "The resulting line chart."
 */
class QUICKCHARTS_EXPORT LineChart : public XYChart
{
    Q_OBJECT
    QML_ELEMENT
    QML_ATTACHED(LineChartAttached)

public:
    explicit LineChart(QQuickItem *parent = nullptr);

    /**
     * Interpolate the values in the chart so that the lines become smoothed.
     */
    Q_PROPERTY(bool interpolate READ interpolate WRITE setInterpolate NOTIFY interpolateChanged)
    bool interpolate() const;
    void setInterpolate(bool newInterpolate);
    Q_SIGNAL void interpolateChanged();
    /**
     * The width of a line in the chart.
     */
    Q_PROPERTY(qreal lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)
    qreal lineWidth() const;
    void setLineWidth(qreal width);
    Q_SIGNAL void lineWidthChanged();
    /**
     * The opacity of the area below a line.
     *
     * The default is 0.0. Note that if fillColorSource is set, this value is
     * ignored.
     */
    Q_PROPERTY(qreal fillOpacity READ fillOpacity WRITE setFillOpacity NOTIFY fillOpacityChanged)
    qreal fillOpacity() const;
    void setFillOpacity(qreal opacity);
    Q_SIGNAL void fillOpacityChanged();
    /**
     * A data source that supplies color values for the line charts' fill area.
     *
     * If this is not set (the default), the normal color source will be used,
     * with the fillOpacity used as its opacity.
     */
    Q_PROPERTY(ChartDataSource *fillColorSource READ fillColorSource WRITE setFillColorSource NOTIFY fillColorSourceChanged)
    ChartDataSource *fillColorSource() const;
    void setFillColorSource(ChartDataSource *newFillColorSource);
    Q_SIGNAL void fillColorSourceChanged();
    /**
     * A delegate that will be placed at each line chart point.
     *
     * When this is not null, the specified component will be used to
     * instantiate an object for each point in the chart. These objects will
     * then be placed centered at positions corresponding to the points on the
     * chart. Each instance will have access to the attached properties of
     * LineChartAttached through LineChart attached object.
     *
     * \note The component assigned to this property is expected to create a
     *       QQuickItem, since the created object needs to be positioned.
     */
    Q_PROPERTY(QQmlComponent *pointDelegate READ pointDelegate WRITE setPointDelegate NOTIFY pointDelegateChanged)
    QQmlComponent *pointDelegate() const;
    void setPointDelegate(QQmlComponent *newPointDelegate);
    Q_SIGNAL void pointDelegateChanged();

    static LineChartAttached *qmlAttachedProperties(QObject *object)
    {
        return new LineChartAttached(object);
    }

protected:
    void updatePolish() override;
    QSGNode *updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *data) override;
    void onDataChanged() override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    void updateLineNode(LineChartNode *node, ChartDataSource *valueSource, const QColor &lineColor, const QColor &fillColor, qreal lineWidth);
    void createPointDelegates(const QList<QVector2D> &values, int sourceIndex);
    void updatePointDelegate(QQuickItem *delegate, const QVector2D &position, const QVariant &value, int sourceIndex);

    bool m_interpolate = false;
    qreal m_lineWidth = 1.0;
    qreal m_fillOpacity = 0.0;
    bool m_rangeInvalid = true;
    ChartDataSource *m_fillColorSource = nullptr;
    QHash<ChartDataSource *, QList<QVector2D>> m_values;
    QQmlComponent *m_pointDelegate = nullptr;
    QHash<ChartDataSource *, QList<QQuickItem *>> m_pointDelegates;
};

#endif // LINECHART_H
