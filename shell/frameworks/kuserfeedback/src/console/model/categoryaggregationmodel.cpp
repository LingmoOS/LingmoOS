/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "categoryaggregationmodel.h"
#include <model/timeaggregationmodel.h>
#include <core/sample.h>

#include <QDebug>
#include <QSet>

#include <algorithm>
#include <string.h>

using namespace KUserFeedback::Console;

CategoryAggregationModel::CategoryAggregationModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

CategoryAggregationModel::~CategoryAggregationModel()
{
    delete[] m_data;
}

void CategoryAggregationModel::setSourceModel(QAbstractItemModel* model)
{
    Q_ASSERT(model);
    m_sourceModel = model;
    connect(model, &QAbstractItemModel::modelReset, this, &CategoryAggregationModel::recompute);
    recompute();
}

void CategoryAggregationModel::setAggregation(const Aggregation& aggr)
{
    m_aggr = aggr;
    m_depth = m_aggr.elements().size();
    recompute();
}

void CategoryAggregationModel::setDepth(int depth)
{
    if (depth == m_depth)
        return;
    m_depth = std::min(depth, (int)m_aggr.elements().size());
    recompute();
}

int CategoryAggregationModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_categories.size() + 1;
}

int CategoryAggregationModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !m_sourceModel)
        return 0;
    return m_sourceModel->rowCount();
}

QVariant CategoryAggregationModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_sourceModel)
        return {};

    if (role == TimeAggregationModel::MaximumValueRole)
        return m_maxValue;

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

QVariant CategoryAggregationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section < 0 || section >= columnCount())
        return {};

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

void CategoryAggregationModel::recompute()
{
    if (!m_sourceModel)
        return;

    const auto rowCount = m_sourceModel->rowCount();
    beginResetModel();
    m_categories.clear();
    delete[] m_data;
    m_data = nullptr;
    m_maxValue = 0;

    if (rowCount <= 0 || !m_aggr.isValid() || m_depth <= 0) {
        endResetModel();
        return;
    }

    // scan all samples to find all categories
    const auto allSamples = m_sourceModel->index(0, 0).data(TimeAggregationModel::AllSamplesRole).value<QVector<Sample>>();
    QVector<QVector<QVector<Sample>>> depthSamples{{allSamples}}; // depth -> parent category index -> samples
    depthSamples.resize(m_depth + 1);
    QVector<QVector<QVector<QString>>> depthCategories{{{{}}}}; // depth -> parent category index -> category values
    depthCategories.resize(m_depth + 1);
    QVector<QVector<int>> depthOffsets{{0}}; // depth -> parent category index -> column offset
    depthOffsets.resize(m_depth + 1);
    for (int i = 0; i < m_depth; ++i) { // for each depth layer...
        depthOffsets[i + 1] = { 0 };
        int prevSize = 0;
        for (int j = 0; j < depthCategories.at(i).size(); ++j) { // ... and for each parent category ...
            for (int k = 0; k < depthCategories.at(i).at(j).size(); ++k) { // ... and for each category value...
                const auto sampleSubSet = depthSamples.at(i).at(depthOffsets.at(i).at(j) + k);
                QHash<QString, QVector<Sample>> catHash;
                for (const auto &s : sampleSubSet) // ... and for each sample
                    catHash[sampleValue(s, i).toString()].push_back(s);
                QVector<QString> cats;
                cats.reserve(catHash.size());
                for (auto it = catHash.cbegin(); it != catHash.cend(); ++it)
                    cats.push_back(it.key());
                std::sort(cats.begin(), cats.end());
                depthCategories[i + 1].push_back(cats);
                for (const auto &cat : qAsConst(cats))
                    depthSamples[i + 1].push_back(catHash.value(cat));
                if (k > 0 || j > 0)
                    depthOffsets[i + 1].push_back(depthOffsets.at(i + 1).constLast() + prevSize);
                prevSize = cats.size();
            }
        }
    }

    for (const auto &cats : depthCategories.at(m_depth))
        m_categories += cats;
    const auto colCount = m_categories.size();

    // compute the counts per cell, we could do that on demand, but we need the maximum for QtCharts...
    m_data = new int[colCount * rowCount];
    memset(m_data, 0, sizeof(int) * colCount * rowCount);
    for (int row = 0; row < rowCount; ++row) {
        const auto samples = m_sourceModel->index(row, 0).data(TimeAggregationModel::SamplesRole).value<QVector<Sample>>();
        for (const auto &sample : samples) {
            int parentIdx = 0;
            for (int i = 1; i <= m_depth; ++i) {
                const auto cats = depthCategories.at(i).at(parentIdx);
                const auto catIt = std::lower_bound(cats.constBegin(), cats.constEnd(), sampleValue(sample, i - 1).toString());
                Q_ASSERT(catIt != cats.constEnd());
                parentIdx = std::distance(cats.constBegin(), catIt) + depthOffsets.at(i).at(parentIdx);
            }
            m_data[colCount * row + parentIdx]++;
        }
        // accumulate per row for stacked plots
        for (int col = 1; col < colCount; ++col) {
            const auto idx = colCount * row + col;
            m_data[idx] += m_data[idx - 1];
        }
        m_maxValue = std::max(m_maxValue, m_data[row * colCount + colCount - 1]);
    }

    endResetModel();
}

QVariant CategoryAggregationModel::sampleValue(const Sample& s, int depth) const
{
    const auto elem = m_aggr.elements().at(depth);
    switch (elem.type()) {
        case AggregationElement::Value:
            return s.value(elem.schemaEntry().name() + QLatin1String(".") + elem.schemaEntryElement().name());
        case AggregationElement::Size:
            const auto l = s.value(elem.schemaEntry().name());
            return l.value<QVariantList>().size();
            break;
    }
    return {};
}

#include "moc_categoryaggregationmodel.cpp"
