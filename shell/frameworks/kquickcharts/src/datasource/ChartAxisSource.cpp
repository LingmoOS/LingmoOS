/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "ChartAxisSource.h"

#include <QDebug>
#include <QVariant>

#include "XYChart.h"

ChartAxisSource::ChartAxisSource(QObject *parent)
    : ChartDataSource(parent)
{
    connect(this, &ChartAxisSource::itemCountChanged, this, &ChartAxisSource::dataChanged);
    connect(this, &ChartAxisSource::chartChanged, this, &ChartAxisSource::dataChanged);
    connect(this, &ChartAxisSource::axisChanged, this, &ChartAxisSource::dataChanged);
}

QVariant ChartAxisSource::item(int index) const
{
    if (!m_chart || index < 0 || index > m_itemCount) {
        return {};
    }

    auto range = m_chart->computedRange();
    if (m_axis == Axis::XAxis) {
        return range.startX + (range.distanceX / (m_itemCount - 1)) * index;
    } else {
        return range.startY + (range.distanceY / (m_itemCount - 1)) * index;
    }
}

QVariant ChartAxisSource::minimum() const
{
    if (!m_chart) {
        return {};
    }

    if (m_axis == Axis::XAxis) {
        return m_chart->computedRange().startX;
    } else {
        return m_chart->computedRange().startY;
    }
}

QVariant ChartAxisSource::maximum() const
{
    if (!m_chart) {
        return {};
    }

    if (m_axis == Axis::XAxis) {
        return m_chart->computedRange().endX;
    } else {
        return m_chart->computedRange().endY;
    }
}

XYChart *ChartAxisSource::chart() const
{
    return m_chart;
}

void ChartAxisSource::setChart(XYChart *newChart)
{
    if (newChart == m_chart) {
        return;
    }

    if (m_chart) {
        disconnect(m_chart, &XYChart::computedRangeChanged, this, &ChartAxisSource::dataChanged);
    }

    m_chart = newChart;
    if (m_chart) {
        connect(m_chart, &XYChart::computedRangeChanged, this, &ChartAxisSource::dataChanged);
    }
    Q_EMIT chartChanged();
}

ChartAxisSource::Axis ChartAxisSource::axis() const
{
    return m_axis;
}

void ChartAxisSource::setAxis(ChartAxisSource::Axis newAxis)
{
    if (newAxis == m_axis) {
        return;
    }

    m_axis = newAxis;
    Q_EMIT axisChanged();
}

int ChartAxisSource::itemCount() const
{
    return m_itemCount;
}

void ChartAxisSource::setItemCount(int newItemCount)
{
    if (newItemCount == m_itemCount) {
        return;
    }

    m_itemCount = newItemCount;
    Q_EMIT itemCountChanged();
}

#include "moc_ChartAxisSource.cpp"
