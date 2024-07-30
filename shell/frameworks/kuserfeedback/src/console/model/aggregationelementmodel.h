/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/


#ifndef KUSERFEEDBACK_CONSOLE_AGGREGATIONELEMENTMODEL_H
#define KUSERFEEDBACK_CONSOLE_AGGREGATIONELEMENTMODEL_H

#include <core/aggregation.h>

#include <QAbstractListModel>
#include <QVector>

namespace KUserFeedback {
namespace Console {

class Product;

class AggregationElementModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit AggregationElementModel(QObject *parent = nullptr);
    ~AggregationElementModel() override;

    void setProduct(const Product &product);

    int rowCount(const QModelIndex & parent) const override;
    QVariant data(const QModelIndex & index, int role) const override;
    QModelIndexList match(const QModelIndex & start, int role, const QVariant & value, int hits, Qt::MatchFlags flags) const override;

private:
    QVector<AggregationElement> m_elements;
};
}
}

#endif // KUSERFEEDBACK_CONSOLE_AGGREGATIONELEMENTMODEL_H
