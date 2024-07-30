/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "numericaggregationmodel.h"
#include <model/timeaggregationmodel.h>
#include <core/sample.h>

using namespace KUserFeedback::Console;

NumericAggregationModel::NumericAggregationModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

NumericAggregationModel::~NumericAggregationModel() = default;

void NumericAggregationModel::setSourceModel(QAbstractItemModel* model)
{
    Q_ASSERT(model);
    m_sourceModel = model;
    connect(model, &QAbstractItemModel::modelReset, this, &NumericAggregationModel::recompute);
    recompute();
}

void NumericAggregationModel::setAggregation(const AggregationElement& aggr)
{
    m_aggr = aggr;
    recompute();
}

int NumericAggregationModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 6;
}

int NumericAggregationModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !m_sourceModel)
        return 0;
    return m_sourceModel->rowCount();
}

QVariant NumericAggregationModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_sourceModel || index.row()>=m_data.count())
        return {};

    if (role == TimeAggregationModel::MaximumValueRole)
        return m_maxValue;

    if (index.column() == 0) {
        const auto srcIdx = m_sourceModel->index(index.row(), 0);
        return m_sourceModel->data(srcIdx, role);
    }
    if (role == Qt::DisplayRole || role == TimeAggregationModel::DataDisplayRole) {
        const auto d = m_data.at(index.row());
        switch (index.column()) {
            case 1: return d.lowerExtreme;
            case 2: return d.lowerQuartile;
            case 3: return d.median;
            case 4: return d.upperQuartile;
            case 5: return d.upperExtreme;
        }
    }

    return {};
}

QVariant NumericAggregationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && m_sourceModel) {
        switch (section) {
            case 0: return m_sourceModel->headerData(section, orientation, role);
            case 1: return tr("Lower Extreme");
            case 2: return tr("Lower Quartile");
            case 3: return tr("Median");
            case 4: return tr("Upper Quartile");
            case 5: return tr("Upper Extreme");
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

void NumericAggregationModel::recompute()
{
    if (!m_sourceModel)
        return;

    const auto rowCount = m_sourceModel->rowCount();
    beginResetModel();

    m_data.clear();
    m_maxValue = 0;

    if (rowCount <= 0 || !m_aggr.isValid()) {
        endResetModel();
        return;
    }

    m_data.reserve(rowCount);
    QVector<double> values;

    for (int row = 0; row < rowCount; ++row) {
        const auto samples = m_sourceModel->index(row, 0).data(TimeAggregationModel::SamplesRole).value<QVector<Sample>>();

        values.clear();
        values.reserve(samples.size());

        foreach (const auto &sample, samples)
            values += sampleValues(sample);

        std::sort(values.begin(), values.end());

        Data d;
        if (values.size() > 0) {
            d.lowerExtreme = values.at(0);
            d.lowerQuartile = values.at(values.size() / 4);
            d.median = values.at(values.size() / 2);
            d.upperQuartile = values.at(values.size() * 3 / 4);
            d.upperExtreme = values.last();
            m_maxValue = std::max(m_maxValue, d.upperExtreme);
        }
        m_data.push_back(d);
    }

    endResetModel();
}

QVector<double> NumericAggregationModel::sampleValues(const Sample &s) const
{
    switch (m_aggr.schemaEntry().dataType()) {
        case SchemaEntry::Scalar:
        {
            if (m_aggr.type() != AggregationElement::Value)
                return {};
            return {s.value(m_aggr.schemaEntry().name() + QLatin1String(".") + m_aggr.schemaEntryElement().name()).toDouble()};
        }
        case SchemaEntry::List:
        {
            const auto l = s.value(m_aggr.schemaEntry().name()).value<QVariantList>();
            if (m_aggr.type() == AggregationElement::Size)
                return {(double)l.size()};
            QVector<double> r;
            r.reserve(l.size());
            for (const auto &entry : l)
                r.push_back(entry.toMap().value(m_aggr.schemaEntryElement().name()).toDouble());
            return r;
        }
        case SchemaEntry::Map:
        {
            const auto m = s.value(m_aggr.schemaEntry().name()).toMap();
            if (m_aggr.type() == AggregationElement::Size)
                return {(double)m.size()};
            QVector<double> r;
            r.reserve(m.size());
            for (auto it = m.begin(); it != m.end(); ++it)
                r.push_back(it.value().toMap().value(m_aggr.schemaEntryElement().name()).toDouble());
            return r;
        }
    }

    return {};
}

#include "moc_numericaggregationmodel.cpp"
