/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "numericaggregator.h"
#include "chartutil.h"

#include <model/numericaggregationmodel.h>
#include <model/timeaggregationmodel.h>

#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBoxPlotSeries>
#include <QtCharts/QChart>
#include <QtCharts/QHBoxPlotModelMapper>
#include <QtCharts/QValueAxis>

using namespace KUserFeedback::Console;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
using namespace QtCharts;
#endif

NumericAggregator::NumericAggregator() = default;
NumericAggregator::~NumericAggregator() = default;

Aggregator::ChartModes NumericAggregator::chartModes() const
{
    Aggregator::ChartModes modes = None;
    if (aggregation().elements().size() == 1)
        modes |= Timeline;
    return modes;
}

QAbstractItemModel* NumericAggregator::timeAggregationModel()
{
    if (!m_model && !aggregation().elements().isEmpty()) {
        m_model.reset(new NumericAggregationModel);
        m_model->setSourceModel(sourceModel());
        const auto e = aggregation().elements().at(0);
        m_model->setAggregation(e);
        QObject::connect(m_model.get(), &QAbstractItemModel::modelReset, [this]() {
            updateTimelineChart();
        });
    }
    return m_model.get();
}

QChart* NumericAggregator::timelineChart()
{
    if (!m_timelineChart) {
        m_timelineChart.reset(new QChart);
        ChartUtil::applyTheme(m_timelineChart.get());
        auto xAxis = new QBarCategoryAxis(m_timelineChart.get());
        auto yAxis = new QValueAxis(m_timelineChart.get());
        yAxis->setMinorTickCount(4);
        m_timelineChart->addAxis(xAxis, Qt::AlignBottom);
        m_timelineChart->addAxis(yAxis, Qt::AlignLeft);
        updateTimelineChart();
    }

    return m_timelineChart.get();
}

void NumericAggregator::updateTimelineChart()
{
    if (!m_timelineChart)
        return;
    m_timelineChart->removeAllSeries();

    auto series = new QBoxPlotSeries(m_timelineChart.get());
    series->setName(displayName());
    auto mapper = new QHBoxPlotModelMapper(series);
    mapper->setModel(timeAggregationModel());
    mapper->setFirstColumn(1);
    mapper->setFirstBoxSetRow(0);
    mapper->setLastBoxSetRow(timeAggregationModel()->rowCount());
    mapper->setSeries(series);
    m_timelineChart->addSeries(series);

    series->attachAxis(m_timelineChart->axisX());
    series->attachAxis(m_timelineChart->axisY());

    QStringList l;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        l.push_back(timeAggregationModel()->index(i, 0).data(TimeAggregationModel::DateTimeRole).toDateTime().toString(QStringLiteral("yyyy-MM-dd")));
    }

    qobject_cast<QBarCategoryAxis*>(m_timelineChart->axisX())->setCategories(l);
    const auto max = timeAggregationModel()->index(0, 0).data(TimeAggregationModel::MaximumValueRole).toInt();
    m_timelineChart->axisY()->setRange(0, max);
    qobject_cast<QValueAxis*>(m_timelineChart->axisY())->applyNiceNumbers();
}
