/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_AGGREGATIONELEMENTEDITMODEL_H
#define KUSERFEEDBACK_CONSOLE_AGGREGATIONELEMENTEDITMODEL_H

#include <core/aggregation.h>

#include <QAbstractListModel>

namespace KUserFeedback {
namespace Console {

class AggregationElementEditModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit AggregationElementEditModel(QObject *parent = nullptr);
    ~AggregationElementEditModel() override;

    Aggregation aggregation() const;
    void setAggregation(const Aggregation &aggregation);

    int rowCount(const QModelIndex & parent) const override;
    QVariant data(const QModelIndex & index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role) override;

private:
    Aggregation m_aggr;
};

}}

#endif // KUSERFEEDBACK_CONSOLE_AGGREGATIONELEMENTEDITMODEL_H
