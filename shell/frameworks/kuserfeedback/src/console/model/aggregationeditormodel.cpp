/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "aggregationeditormodel.h"

#include <core/aggregation.h>
#include <core/util.h>

using namespace KUserFeedback::Console;

AggregationEditorModel::AggregationEditorModel(QObject* parent) :
    QAbstractTableModel(parent)
{
}

AggregationEditorModel::~AggregationEditorModel() = default;

Product AggregationEditorModel::product() const
{
    return m_product;
}

void AggregationEditorModel::setProduct(const Product& product)
{
    beginResetModel();
    m_product = product;
    endResetModel();
}

int AggregationEditorModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 3;
}

int AggregationEditorModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_product.aggregations().size();
}

QVariant AggregationEditorModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_product.isValid())
        return {};

    if (role == Qt::DisplayRole) {
        const auto aggr = m_product.aggregations().at(index.row());
        switch (index.column()) {
            case 0:
                return aggr.name();
            case 1:
                return Util::enumToString(aggr.type());
            case 2:
            {
                if (aggr.elements().isEmpty())
                    return tr("<none>");
                QStringList l;
                l.reserve(aggr.elements().size());
                foreach (const auto &elem, aggr.elements())
                    l.push_back(elem.displayString());
                return l.join(QStringLiteral(", "));
            }
        }
    } else if (role == Qt::EditRole) {
        const auto aggr = m_product.aggregations().at(index.row());
        switch (index.column()) {
            case 0:
                return aggr.name();
            case 1:
                return QVariant::fromValue(aggr.type());
            case 2:
                if (aggr.elements().isEmpty())
                    return QVariant::fromValue(AggregationElement());
                return QVariant::fromValue(aggr.elements().at(0));
        }
    }

    return {};
}

QVariant AggregationEditorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return tr("Name");
            case 1: return tr("Type");
            case 2: return tr("Element");
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags AggregationEditorModel::flags(const QModelIndex& index) const
{
    const auto baseFlags = QAbstractTableModel::flags(index);
    if (!index.isValid())
        return baseFlags;

    const auto aggr = m_product.aggregations().at(index.row());
    if (aggr.type() == Aggregation::Category && index.column() == 2)
        return baseFlags;

    return baseFlags | Qt::ItemIsEditable;
}

bool AggregationEditorModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    auto aggrs = m_product.aggregations();
    auto &aggr = aggrs[index.row()];
    switch (index.column()) {
        case 0:
            aggr.setName(value.toString());
            break;
        case 1:
            aggr.setType(value.value<Aggregation::Type>());
            break;
        case 2:
            aggr.setElements({ value.value<AggregationElement>() });
            break;
    }
    m_product.setAggregations(aggrs);
    Q_EMIT dataChanged(index, index);
    return true;
}

#include "moc_aggregationeditormodel.cpp"
