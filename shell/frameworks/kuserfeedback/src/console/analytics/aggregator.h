/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_AGGREGATOR_H
#define KUSERFEEDBACK_CONSOLE_AGGREGATOR_H

#include <core/aggregation.h>

#include <memory>

class QAbstractItemModel;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
namespace QtCharts {
class QChart;
}
using QtCharts::QChart;
#else
class QChart;
#endif

namespace KUserFeedback {
namespace Console {

class Aggregation;
class SingleRowFilterProxyModel;

class Aggregator
{
public:
    explicit Aggregator();
    virtual ~Aggregator();

    enum ChartMode {
        None = 0x0,
        Singular = 0x1,
        Timeline = 0x2
    };
    Q_DECLARE_FLAGS(ChartModes, ChartMode)

    Aggregation aggregation() const;
    void setAggregation(const Aggregation &aggr);

    QAbstractItemModel *sourceModel() const;
    void setSourceModel(QAbstractItemModel *model);

    virtual QString displayName() const;

    virtual ChartModes chartModes() const;

    virtual QAbstractItemModel *timeAggregationModel();
    QAbstractItemModel* singularAggregationModel();
    virtual void setSingularTime(int row);

    virtual QChart *singlularChart();
    virtual QChart *timelineChart();

private:
    QAbstractItemModel *m_sourceModel = nullptr;
    std::unique_ptr<SingleRowFilterProxyModel> m_singularModel;
    Aggregation m_aggregation;
};

}}

Q_DECLARE_METATYPE(KUserFeedback::Console::Aggregator*)
Q_DECLARE_OPERATORS_FOR_FLAGS(KUserFeedback::Console::Aggregator::ChartModes)

#endif // KUSERFEEDBACK_CONSOLE_AGGREGATOR_H
