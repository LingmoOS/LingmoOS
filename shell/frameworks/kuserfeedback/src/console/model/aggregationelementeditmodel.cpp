/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "aggregationelementeditmodel.h"

using namespace KUserFeedback::Console;

AggregationElementEditModel::AggregationElementEditModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

AggregationElementEditModel::~AggregationElementEditModel() = default;

Aggregation AggregationElementEditModel::aggregation() const
{
    return m_aggr;
}

void AggregationElementEditModel::setAggregation(const Aggregation& aggregation)
{
    beginResetModel();
    m_aggr = aggregation;
    endResetModel();
}

int AggregationElementEditModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_aggr.elements().size();
}

QVariant AggregationElementEditModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    switch (role) {
        case Qt::DisplayRole:
        {
            const auto elem = m_aggr.elements().at(index.row());
            if (elem.isValid())
                return elem.displayString();
            return tr("<new element>");
        }
        case Qt::EditRole:
            return QVariant::fromValue(m_aggr.elements().at(index.row()));
    }

    return {};
}

QVariant AggregationElementEditModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0)
        return tr("Element");
    return QAbstractListModel::headerData(section, orientation, role);
}

Qt::ItemFlags AggregationElementEditModel::flags(const QModelIndex& index) const
{
    const auto baseFlags = QAbstractListModel::flags(index);
    return baseFlags | Qt::ItemIsEditable;
}

bool AggregationElementEditModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole || value.isNull())
        return false;

    auto elems = m_aggr.elements();
    elems[index.row()] = value.value<AggregationElement>();
    m_aggr.setElements(elems);
    Q_EMIT dataChanged(index, index);
    return true;
}

#include "moc_aggregationelementeditmodel.cpp"
