/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_NUMERICAGGREGATOR_H
#define KUSERFEEDBACK_CONSOLE_NUMERICAGGREGATOR_H

#include "aggregator.h"

#include <memory>

namespace KUserFeedback {
namespace Console {

class NumericAggregationModel;

class NumericAggregator :  public Aggregator
{
public:
    NumericAggregator();
    ~NumericAggregator() override;

    ChartModes chartModes() const override;
    QAbstractItemModel* timeAggregationModel() override;
    QChart* timelineChart() override;

private:
    void updateTimelineChart();

    std::unique_ptr<NumericAggregationModel> m_model;
    std::unique_ptr<QChart> m_timelineChart;
};

}}

#endif // KUSERFEEDBACK_CONSOLE_NUMERICAGGREGATOR_H
