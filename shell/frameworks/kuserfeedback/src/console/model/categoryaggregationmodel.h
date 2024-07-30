/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_CATEGORYAGGREGATIONMODEL_H
#define KUSERFEEDBACK_CONSOLE_CATEGORYAGGREGATIONMODEL_H

#include <core/aggregation.h>

#include <QAbstractTableModel>
#include <QVector>

namespace KUserFeedback {
namespace Console {

class Sample;

/** Aggregate by time and one string category value (e.g. version. platform, etc). */
class CategoryAggregationModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit CategoryAggregationModel(QObject *parent = nullptr);
    ~CategoryAggregationModel() override;

    void setSourceModel(QAbstractItemModel *model);
    void setAggregation(const Aggregation &aggr);
    /*! Limits depth to @p depth, even if the aggregation has a higher one. */
    void setDepth(int depth);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    void recompute();
    QVariant sampleValue(const Sample &s, int depth) const;

    QAbstractItemModel *m_sourceModel = nullptr;
    Aggregation m_aggr;
    QVector<QString> m_categories;
    int *m_data = nullptr;
    int m_maxValue;
    int m_depth;
};

}
}

#endif // KUSERFEEDBACK_CONSOLE_CATEGORYAGGREGATIONMODEL_H
