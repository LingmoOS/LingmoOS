/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "categoryaggregator.h"
#include "chartutil.h"

#include <model/categoryaggregationmodel.h>
#include <model/extrarowsproxymodel.h>
#include <model/rolemappingproxymodel.h>
#include <model/singlerowfilterproxymodel.h>
#include <model/timeaggregationmodel.h>

#include <QtCharts/QAreaSeries>
#include <QtCharts/QChart>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QHPieModelMapper>
#include <QtCharts/QLineSeries>
#include <QtCharts/QPieSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QVXYModelMapper>

#include <QDebug>

using namespace KUserFeedback::Console;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
using namespace QtCharts;
#endif

CategoryAggregator::CategoryAggregator() = default;
CategoryAggregator::~CategoryAggregator() = default;

Aggregator::ChartModes CategoryAggregator::chartModes() const
{
    Aggregator::ChartModes modes = None;
    if (aggregation().elements().size() == 1)
        modes |= Timeline;
    if (aggregation().elements().size() >= 1)
        modes |= Singular;
    return modes;
}

QAbstractItemModel* CategoryAggregator::timeAggregationModel()
{
    if (!m_model && !aggregation().elements().isEmpty()) {
        m_model.reset(new CategoryAggregationModel);
        m_model->setSourceModel(sourceModel());
        m_model->setAggregation(aggregation());
        QObject::connect(m_model.get(), &QAbstractItemModel::modelReset, [this]() {
            updateSingularChart();
            updateTimelineChart();
        });
    }
    return m_model.get();
}

QChart* CategoryAggregator::timelineChart()
{
    if (!m_timelineChart) {
        m_timelineChart.reset(new QChart);
        ChartUtil::applyTheme(m_timelineChart.get());
        auto xAxis = new QDateTimeAxis(m_timelineChart.get());
        xAxis->setFormat(QStringLiteral("yyyy-MM-dd")); // TODO, follow aggregation mode
        auto yAxis = new QValueAxis(m_timelineChart.get());
        xAxis->setTickCount(std::min(timeAggregationModel()->rowCount(), 12));
        yAxis->setMinorTickCount(4);

        m_timelineChart->addAxis(xAxis, Qt::AlignBottom);
        m_timelineChart->addAxis(yAxis, Qt::AlignLeft);
        updateTimelineChart();
    }

    return m_timelineChart.get();
}

void CategoryAggregator::updateTimelineChart()
{
    if (!m_timelineChart)
        return;
    m_timelineChart->removeAllSeries();

    QLineSeries *prevSeries = nullptr;
    auto model = new RoleMappingProxyModel(m_timelineChart.get());
    model->setSourceModel(timeAggregationModel());
    model->addRoleMapping(Qt::DisplayRole, TimeAggregationModel::AccumulatedDisplayRole);
    for (int i = 1; i < timeAggregationModel()->columnCount(); ++i) {
        auto series = new QLineSeries;

        auto mapper = new QVXYModelMapper(series);
        mapper->setModel(model);
        mapper->setXColumn(0);
        mapper->setYColumn(i);
        mapper->setFirstRow(0);
        mapper->setSeries(series);

        auto areaSeries = new QAreaSeries;
        series->setParent(areaSeries); // otherwise series isn't deleted by removeAllSeries!
        areaSeries->setLowerSeries(prevSeries);
        areaSeries->setUpperSeries(series);
        areaSeries->setName(timeAggregationModel()->headerData(i, Qt::Horizontal).toString().toHtmlEscaped());
        m_timelineChart->addSeries(areaSeries);

        areaSeries->attachAxis(m_timelineChart->axisX());
        areaSeries->attachAxis(m_timelineChart->axisY());

        prevSeries = series;
    }

    const auto max = timeAggregationModel()->index(0, 0).data(TimeAggregationModel::MaximumValueRole).toInt();
    m_timelineChart->axisY()->setRange(0, max);
    qobject_cast<QValueAxis*>(m_timelineChart->axisY())->applyNiceNumbers();
}

QChart* CategoryAggregator::singlularChart()
{
    if (!m_singularChart) {
        m_singularChart.reset(new QChart);
        ChartUtil::applyTheme(m_singularChart.get());
        updateSingularChart();
    }

    return m_singularChart.get();
}

void CategoryAggregator::setSingularTime(int row)
{
    Aggregator::setSingularTime(row);
    for (auto model : m_hierachicalCategories)
        model->setRow(row);
}

void CategoryAggregator::updateSingularChart()
{
    if (!m_singularChart)
        return;
    m_singularChart->removeAllSeries();
    m_hierachicalCategories.clear();

    if (sourceModel()->rowCount() <= 0)
        return;

    const auto depth = aggregation().elements().size();
    for (int i = 0; i < depth; ++i) {
        auto series = new QPieSeries(m_singularChart.get());
        auto mapper = new QHPieModelMapper(m_singularChart.get());
        auto modelWithLabels = new ExtraRowsProxyModel(mapper);
        if (i + 1 == depth) {
            modelWithLabels->setSourceModel(singularAggregationModel());
        } else {
            auto model = new SingleRowFilterProxyModel(mapper);
            auto catModel = new CategoryAggregationModel(mapper);
            catModel->setSourceModel(sourceModel());
            catModel->setAggregation(aggregation());
            catModel->setDepth(i + 1);
            model->setSourceModel(catModel);
            m_hierachicalCategories.push_back(model);
            modelWithLabels->setSourceModel(model);
        }
        mapper->setModel(modelWithLabels);
        mapper->setFirstColumn(1);
        mapper->setValuesRow(0);
        mapper->setLabelsRow(1);
        mapper->setSeries(series);

        decorateSeries(series, i);
        QObject::connect(series, &QPieSeries::added, [this, series, i]() {
            decorateSeries(series, i);
        });

        m_singularChart->addSeries(series);
    }
}

void CategoryAggregator::decorateSeries(QPieSeries* series, int ring) const
{
    const auto ringCount = aggregation().elements().size();
    const auto ringWidth = 0.7 / (ringCount + 1);
    const auto holeSize = ringWidth * (ringCount - ring);
    series->setPieSize(holeSize + ringWidth);
    series->setHoleSize(holeSize);

    for (auto slice : series->slices()) {
        if (slice->value() > 0.01)
            slice->setLabelVisible(true);
    }
}
