/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_ANALYTICSVIEW_H
#define KUSERFEEDBACK_CONSOLE_ANALYTICSVIEW_H

#include <core/product.h>

#include <QVector>
#include <QWidget>

#include <memory>

class QAbstractItemModel;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
namespace QtCharts {
class QChart;
class QChartView;
}
using QtCharts::QChart;
using QtCharts::QChartView;
#else
class QChart;
#endif

namespace KUserFeedback {
namespace Console {

class Aggregator;
class AggregatedDataModel;
class DataModel;
class RESTClient;
class TimeAggregationModel;

namespace Ui
{
class AnalyticsView;
}

class AnalyticsView : public QWidget
{
    Q_OBJECT
public:
    explicit AnalyticsView(QWidget *parent = nullptr);
    ~AnalyticsView() override;

    void setProduct(const Product &product);
    void setRESTClient(RESTClient *client);

Q_SIGNALS:
    void logMessage(const QString &msg);

private:
    void chartSelected();
    void updateChart();
    void updateTimeSliderRange();

    Aggregator* createAggregator(const Aggregation &aggr) const;

    void exportData();
    void importData();
    void exportChart();

    std::unique_ptr<Ui::AnalyticsView> ui;

    RESTClient *m_client = nullptr;
    DataModel *m_dataModel;
    TimeAggregationModel *m_timeAggregationModel;
    AggregatedDataModel *m_aggregatedDataModel;

    QVector<Aggregator*> m_aggregators;

    std::unique_ptr<QChart> m_nullSingularChart;
    std::unique_ptr<QChart> m_nullTimelineChart;
};
}
}

#endif // KUSERFEEDBACK_CONSOLE_ANALYTICSVIEW_H
