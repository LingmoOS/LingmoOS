/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "aggregator.h"

#include <model/singlerowfilterproxymodel.h>

using namespace KUserFeedback::Console;

Aggregator::Aggregator() = default;
Aggregator::~Aggregator() = default;

Aggregation Aggregator::aggregation() const
{
    return m_aggregation;
}

void Aggregator::setAggregation(const Aggregation& aggr)
{
    m_aggregation = aggr;
}

QAbstractItemModel* Aggregator::sourceModel() const
{
    return m_sourceModel;
}

void Aggregator::setSourceModel(QAbstractItemModel* model)
{
    m_sourceModel = model;
}

QString Aggregator::displayName() const
{
    return m_aggregation.name();
}

Aggregator::ChartModes Aggregator::chartModes() const
{
    return None;
}

QAbstractItemModel* Aggregator::timeAggregationModel()
{
    return nullptr;
}

QAbstractItemModel* Aggregator::singularAggregationModel()
{
    if (!m_singularModel) {
        m_singularModel.reset(new SingleRowFilterProxyModel);
        m_singularModel->setSourceModel(timeAggregationModel());
    }
    return m_singularModel.get();
}

void Aggregator::setSingularTime(int row)
{
    singularAggregationModel();
    m_singularModel->setRow(row);
}

QChart *Aggregator::singlularChart()
{
    return nullptr;
}

QChart *Aggregator::timelineChart()
{
    return nullptr;
}
