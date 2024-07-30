/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "ratiosetaggregationmodel.h"
#include <model/timeaggregationmodel.h>
#include <core/sample.h>

#include <QSet>

using namespace KUserFeedback::Console;

RatioSetAggregationModel::RatioSetAggregationModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

RatioSetAggregationModel::~RatioSetAggregationModel()
{
    delete[] m_data;
}

void RatioSetAggregationModel::setSourceModel(QAbstractItemModel* model)
{
    Q_ASSERT(model);
    m_sourceModel = model;
    connect(model, &QAbstractItemModel::modelReset, this, &RatioSetAggregationModel::recompute);
    recompute();
}

void RatioSetAggregationModel::setAggregationValue(const QString& aggrValue)
{
    m_aggrValue = aggrValue;
    recompute();
}

int RatioSetAggregationModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_categories.size() + 1;
}

int RatioSetAggregationModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !m_sourceModel)
        return 0;
    return m_sourceModel->rowCount();
}

QVariant RatioSetAggregationModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_sourceModel)
        return {};

    if (role == TimeAggregationModel::MaximumValueRole)
        return 1.0;

    if (index.column() == 0) {
        const auto srcIdx = m_sourceModel->index(index.row(), 0);
        return m_sourceModel->data(srcIdx, role);
    }

    const auto idx = index.row() * m_categories.size() + index.column() - 1;
    switch (role) {
        case TimeAggregationModel::AccumulatedDisplayRole:
            return m_data[idx];
        case Qt::DisplayRole:
        case TimeAggregationModel::DataDisplayRole:
            if (index.column() == 1)
                return m_data[idx];
            return m_data[idx] - m_data[idx - 1];
    }

    return {};
}

QVariant RatioSetAggregationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && m_sourceModel) {
        if (section == 0)
            return m_sourceModel->headerData(section, orientation, role);
        if (role == Qt::DisplayRole) {
            const auto cat = m_categories.at(section - 1);
            if (cat.isEmpty())
                return tr("[empty]");
            return cat;
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

void RatioSetAggregationModel::recompute()
{
    if (!m_sourceModel)
        return;

    const auto rowCount = m_sourceModel->rowCount();
    beginResetModel();
    m_categories.clear();
    delete[] m_data;
    m_data = nullptr;

    if (rowCount <= 0 || m_aggrValue.isEmpty()) {
        endResetModel();
        return;
    }

    QSet<QString> categories;
    const auto allSamples = m_sourceModel->index(0, 0).data(TimeAggregationModel::AllSamplesRole).value<QVector<Sample>>();
    foreach (const auto &s, allSamples) {
        const auto rs = s.value(m_aggrValue).value<QVariantMap>();
        for (auto it = rs.begin(); it != rs.end(); ++it)
            categories.insert(it.key());
    }
    m_categories.reserve(categories.size());
    foreach (const auto &cat, categories)
        m_categories.push_back(cat);
    std::sort(m_categories.begin(), m_categories.end());
    const auto colCount = m_categories.size();

    // compute the counts per cell, we could do that on demand, but we need the maximum for QtCharts...
    m_data = new double[colCount * rowCount];
    auto rowData = new double[colCount];
    memset(m_data, 0, sizeof(double) * colCount * rowCount);
    for (int row = 0; row < rowCount; ++row) {
        const auto samples = m_sourceModel->index(row, 0).data(TimeAggregationModel::SamplesRole).value<QVector<Sample>>();
        int validSampleCount = 0;
        foreach (const auto &sample, samples) {
            // extract raw data for one sample
            memset(rowData, 0, sizeof(double) * colCount);
            const auto rs = sample.value(m_aggrValue).value<QVariantMap>();
            for (auto it = rs.begin(); it != rs.end(); ++it) {
                const auto catIt = std::lower_bound(m_categories.constBegin(), m_categories.constEnd(), it.key());
                Q_ASSERT(catIt != m_categories.constEnd());
                const auto idx = std::distance(m_categories.constBegin(), catIt);
                rowData[idx] += it.value().toMap().value(QLatin1String("property")).toDouble(); // TODO: make this configurable
            }

            // filter invalid samples, normalize to 1
            // TODO: deal with negative values
            const auto sampleSum = std::accumulate(rowData, rowData + colCount, 0.0);
            if (sampleSum <= 0.0)
                continue;
            for (int i = 0; i < colCount; ++i)
                rowData[i] /= sampleSum;

            // aggregate
            ++validSampleCount;
            for (int i = 0; i < colCount; ++i)
                m_data[row * colCount + i] += rowData[i];
        }

        // normalize to 1 and accumulate per row for stacked plots
        if (!validSampleCount)
            continue;
        const double scale = validSampleCount;
        for (int col = 0; col < colCount; ++col) {
            const auto idx = colCount * row + col;
            m_data[idx] /= scale;
            if (col > 0)
                m_data[idx] += m_data[idx - 1];
        }

        Q_ASSERT(m_data[row * colCount + colCount - 1] <= 1.0001);
    }

    delete[] rowData;
    endResetModel();
}

#include "moc_ratiosetaggregationmodel.cpp"
