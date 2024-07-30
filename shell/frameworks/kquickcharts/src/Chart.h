/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef CHART_H
#define CHART_H

#include <QQuickItem>
#include <qqmlregistration.h>

#include "datasource/ChartDataSource.h"

#include "quickcharts_export.h"

/**
 * Abstract base class for all charts.
 */
class QUICKCHARTS_EXPORT Chart : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Base class")

public:
    using DataSourcesProperty = QQmlListProperty<ChartDataSource>;

    /**
     * How to index color and name sources relative to the different value sources.
     */
    enum IndexingMode {
        IndexSourceValues = 1, ///< Index each value, restart indexing for each value source.
        IndexEachSource, ///< Index each value source, never index individual values.
        IndexAllValues ///< Index each value, continuing with the index for each value source.
    };
    Q_ENUM(IndexingMode)

    explicit Chart(QQuickItem *parent = nullptr);
    ~Chart() override = default;

    /**
     * The data source to use for names of chart items.
     */
    Q_PROPERTY(ChartDataSource *nameSource READ nameSource WRITE setNameSource NOTIFY nameSourceChanged)
    ChartDataSource *nameSource() const;
    void setNameSource(ChartDataSource *nameSource);
    Q_SIGNAL void nameSourceChanged();

    /**
     * The data source to use for short names of chart items.
     */
    Q_PROPERTY(ChartDataSource *shortNameSource READ shortNameSource WRITE setShortNameSource NOTIFY shortNameSourceChanged)
    ChartDataSource *shortNameSource() const;
    void setShortNameSource(ChartDataSource *shortNameSource);
    Q_SIGNAL void shortNameSourceChanged();

    /**
     * The data source to use for colors of chart items.
     */
    Q_PROPERTY(ChartDataSource *colorSource READ colorSource WRITE setColorSource NOTIFY colorSourceChanged)
    ChartDataSource *colorSource() const;
    void setColorSource(ChartDataSource *colorSource);
    Q_SIGNAL void colorSourceChanged();

    /**
     * The data sources providing the data this chart needs to render.
     */
    Q_PROPERTY(QQmlListProperty<ChartDataSource> valueSources READ valueSourcesProperty NOTIFY valueSourcesChanged)
    DataSourcesProperty valueSourcesProperty();
    QList<ChartDataSource *> valueSources() const;
    Q_SIGNAL void valueSourcesChanged();
    Q_INVOKABLE void insertValueSource(int index, ChartDataSource *source);
    Q_INVOKABLE void removeValueSource(int index);
    Q_INVOKABLE void removeValueSource(QObject *source);

    /**
     * The indexing mode used for indexing colors and names.
     */
    Q_PROPERTY(IndexingMode indexingMode READ indexingMode WRITE setIndexingMode NOTIFY indexingModeChanged)
    IndexingMode indexingMode() const;
    void setIndexingMode(IndexingMode newIndexingMode);
    Q_SIGNAL void indexingModeChanged();

    /**
     * The index of a value source to highlight.
     *
     * Highlighting is dependant on Chart type, but will generally mean that
     * other value sources are rendered with lower opacity.
     *
     * By default, this is -1 which means nothing is highlighted.
     */
    Q_PROPERTY(int highlight READ highlight WRITE setHighlight NOTIFY highlightChanged RESET resetHighlight)
    int highlight() const;
    void setHighlight(int highlight);
    void resetHighlight();
    Q_SIGNAL void highlightChanged();

    Q_SIGNAL void dataChanged();

protected:
    /**
     * Called when the data of a value source changes.
     *
     * This method should be reimplemented by subclasses. It is called whenever
     * the data of one of the value sources changes. Subclasses should use this
     * to make sure that they update whatever internal state they use for
     * rendering, then call update() to schedule rendering the item.
     */
    virtual void onDataChanged() = 0;
    void componentComplete() override;

    /**
     * Desaturate and de-emphasise a color.
     *
     * Mainly intended as a standard for ensuring everything but the highlighted
     * item is desaturated.
     */
    QColor desaturate(const QColor &input);

private:
    static void appendSource(DataSourcesProperty *list, ChartDataSource *source);
    static qsizetype sourceCount(DataSourcesProperty *list);
    static ChartDataSource *source(DataSourcesProperty *list, qsizetype index);
    static void clearSources(DataSourcesProperty *list);
    static void replaceSource(DataSourcesProperty *list, qsizetype index, ChartDataSource *source);
    static void removeLastSource(DataSourcesProperty *list);

    ChartDataSource *m_nameSource = nullptr;
    ChartDataSource *m_shortNameSource = nullptr;
    ChartDataSource *m_colorSource = nullptr;
    QList<ChartDataSource *> m_valueSources;
    IndexingMode m_indexingMode = IndexEachSource;
    int m_highlight = -1;
};

#endif // CHART_H
