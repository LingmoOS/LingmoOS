/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_NUMERICAGGREGATIONMODEL_H
#define KUSERFEEDBACK_CONSOLE_NUMERICAGGREGATIONMODEL_H

#include <core/aggregationelement.h>

#include <QAbstractTableModel>

namespace KUserFeedback {
namespace Console {

class Sample;

class NumericAggregationModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit NumericAggregationModel(QObject *parent = nullptr);
    ~NumericAggregationModel() override;

    void setSourceModel(QAbstractItemModel *model);
    void setAggregation(const AggregationElement &aggr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    void recompute();
    QVector<double> sampleValues(const Sample &s) const;

    QAbstractItemModel *m_sourceModel = nullptr;
    AggregationElement m_aggr;
    struct Data {
        double lowerExtreme = 0.0;
        double lowerQuartile = 0.0;
        double median = 0.0;
        double upperQuartile = 0.0;
        double upperExtreme = 0.0;
    };
    QVector<Data> m_data;

    double m_maxValue = 0.0;
};

}
}

#endif // KUSERFEEDBACK_CONSOLE_NUMERICAGGREGATIONMODEL_H
