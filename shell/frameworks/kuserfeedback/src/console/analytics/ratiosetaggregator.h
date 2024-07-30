/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_RATIOSETAGGREGATOR_H
#define KUSERFEEDBACK_CONSOLE_RATIOSETAGGREGATOR_H

#include "aggregator.h"

#include <memory>

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

class RatioSetAggregationModel;

class RatioSetAggregator : public Aggregator
{
public:
    RatioSetAggregator();
    ~RatioSetAggregator() override;

    ChartModes chartModes() const override;
    QAbstractItemModel* timeAggregationModel() override;
    QChart* timelineChart() override;
    QChart* singlularChart() override;

private:
    void updateTimelineChart();
    void updateSingularChart();

    void decoratePieSeries(QPieSeries *series) const;

    std::unique_ptr<RatioSetAggregationModel> m_model;
    std::unique_ptr<QChart> m_timelineChart;
    std::unique_ptr<QChart> m_singularChart;
};

}}

#endif // KUSERFEEDBACK_CONSOLE_RATIOSETAGGREGATOR_H
