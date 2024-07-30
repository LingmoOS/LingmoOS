/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_TIMEAGGREGATIONMODEL_H
#define KUSERFEEDBACK_CONSOLE_TIMEAGGREGATIONMODEL_H

#include <QAbstractTableModel>
#include <QDateTime>
#include <QVector>

namespace KUserFeedback {
namespace Console {

class Sample;

class TimeAggregationModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum AggregationMode {
        AggregateYear,
        AggregateMonth,
        AggregateWeek,
        AggregateDay
    };

    enum Role {
        DateTimeRole = Qt::UserRole + 1,
        MaximumValueRole,
        TimeDisplayRole,
        DataDisplayRole,
        AccumulatedDisplayRole,
        SamplesRole,
        AllSamplesRole
    };

    explicit TimeAggregationModel(QObject *parent = nullptr);
    ~TimeAggregationModel() override;

    void setSourceModel(QAbstractItemModel *model);

    AggregationMode aggregationMode() const;
    void setAggregationMode(AggregationMode mode);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    void reload();
    QDateTime aggregate(const QDateTime &dt) const;
    QString timeToString(const QDateTime &dt) const;

    QAbstractItemModel *m_sourceModel = nullptr;
    struct Data {
        QDateTime time;
        QVector<Sample> samples;
    };
    QVector<Data> m_data;
    int m_maxValue = 0;
    AggregationMode m_mode = AggregateYear;
};

}
}

#endif // KUSERFEEDBACK_CONSOLE_TIMEAGGREGATIONMODEL_H
