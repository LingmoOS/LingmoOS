/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_TOTALAGGREGATOR_H
#define KUSERFEEDBACK_CONSOLE_TOTALAGGREGATOR_H

#include "aggregator.h"

#include <QCoreApplication>

#include <memory>

namespace KUserFeedback {
namespace Console {

class TotalAggregator : public Aggregator
{
    Q_DECLARE_TR_FUNCTIONS(KUserFeedback::Console::TotalAggregator)
public:
    TotalAggregator();
    ~TotalAggregator() override;

    ChartModes chartModes() const override;
    QString displayName() const override;
    QAbstractItemModel* timeAggregationModel() override;
    QChart* timelineChart() override;

private:
    void updateTimelineChart();

    std::unique_ptr<QChart> m_timelineChart;
};

}}

#endif // KUSERFEEDBACK_CONSOLE_TOTALAGGREGATOR_H
