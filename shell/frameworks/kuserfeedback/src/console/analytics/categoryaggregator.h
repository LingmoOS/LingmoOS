/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_CATEGORYAGGREGATOR_H
#define KUSERFEEDBACK_CONSOLE_CATEGORYAGGREGATOR_H

#include "aggregator.h"

#include <memory>
#include <vector>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
namespace QtCharts {
class QPieSeries;
}
using QtCharts::QPieSeries;
#else
class QPieSeries;
#endif

namespace KUserFeedback {
namespace Console {

class CategoryAggregationModel;
class SingleRowFilterProxyModel;

class CategoryAggregator : public Aggregator
{
public:
    CategoryAggregator();
    ~CategoryAggregator() override;

    ChartModes chartModes() const override;
    QAbstractItemModel* timeAggregationModel() override;
    QChart* timelineChart() override;
    QChart* singlularChart() override;
    void setSingularTime(int row) override;

private:
    void updateTimelineChart();
    void updateSingularChart();
    void decorateSeries(QPieSeries *series, int ring) const;

    std::unique_ptr<CategoryAggregationModel> m_model;
    std::unique_ptr<QChart> m_timelineChart;
    std::unique_ptr<QChart> m_singularChart;
    std::vector<SingleRowFilterProxyModel*> m_hierachicalCategories;
};

}}

#endif // KUSERFEEDBACK_CONSOLE_CATEGORYAGGREGATOR_H
