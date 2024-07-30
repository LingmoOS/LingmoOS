/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "HistoryProxySource.h"

#include <QDebug>

HistoryProxySource::HistoryProxySource(QObject *parent)
    : ChartDataSource(parent)
{
}

int HistoryProxySource::itemCount() const
{
    if (m_fillMode == DoNotFill) {
        return m_history.size();
    } else {
        return m_maximumHistory;
    }
}

QVariant HistoryProxySource::item(int index) const
{
    if (index < 0 || !m_dataSource || m_dataSource->itemCount() == 0) {
        return QVariant{};
    }

    if (m_fillMode == DoNotFill && index >= m_history.count()) {
        return QVariant{};
    }

    if (m_fillMode == FillFromStart && index >= m_history.count()) {
        return QVariant{QMetaType(m_dataSource->item(0).userType())};
    }

    if (m_fillMode == FillFromEnd && m_history.count() != m_maximumHistory) {
        auto actualIndex = index - (m_maximumHistory - m_history.count());
        if (actualIndex < 0 || actualIndex >= m_history.size()) {
            return QVariant{QMetaType(m_dataSource->item(0).userType())};
        } else {
            return m_history.at(actualIndex);
        }
    }

    if (index < m_history.count()) {
        return m_history.at(index);
    } else {
        return QVariant{};
    }
}

QVariant HistoryProxySource::minimum() const
{
    if (m_history.isEmpty() || !m_dataSource) {
        return QVariant{};
    }

    // TODO: Find a nicer solution for data sources to indicate
    // "I provide a min/max value not derived from my items"
    auto model = m_dataSource->property("model").value<QObject *>();
    if (model) {
        auto minProperty = model->property("minimum");
        auto maxProperty = model->property("maximum");
        if (minProperty.isValid() && minProperty != maxProperty) {
            return minProperty;
        }
    }

    return *std::min_element(m_history.begin(), m_history.end(), variantCompare);
}

QVariant HistoryProxySource::maximum() const
{
    if (m_history.isEmpty() || !m_dataSource) {
        return QVariant{};
    }

    auto model = m_dataSource->property("model").value<QObject *>();
    if (model) {
        auto minProperty = model->property("minimum");
        auto maxProperty = model->property("maximum");
        if (maxProperty.isValid() && maxProperty != minProperty) {
            return maxProperty;
        }
    }

    return *std::max_element(m_history.begin(), m_history.end(), variantCompare);
}

QVariant HistoryProxySource::first() const
{
    if (!m_history.isEmpty()) {
        return m_history.first();
    }
    return QVariant{};
}

ChartDataSource *HistoryProxySource::source() const
{
    return m_dataSource;
}

void HistoryProxySource::setSource(ChartDataSource *newSource)
{
    if (newSource == m_dataSource) {
        return;
    }

    if (m_dataSource) {
        m_dataSource->disconnect(this);
    }

    m_dataSource = newSource;
    clear();
    if (m_dataSource) {
        connect(m_dataSource, &ChartDataSource::dataChanged, this, [this]() {
            if (!m_updateTimer) {
                update();
            }
        });
    }
    Q_EMIT sourceChanged();
}

int HistoryProxySource::item() const
{
    return m_item;
}

void HistoryProxySource::setItem(int newItem)
{
    if (newItem == m_item) {
        return;
    }

    m_item = newItem;
    clear();
    Q_EMIT itemChanged();
}

int HistoryProxySource::maximumHistory() const
{
    return m_maximumHistory;
}

void HistoryProxySource::setMaximumHistory(int newMaximumHistory)
{
    if (newMaximumHistory == m_maximumHistory) {
        return;
    }

    m_maximumHistory = newMaximumHistory;
    while (m_history.size() > 0 && m_history.size() > m_maximumHistory) {
        m_history.removeLast();
    }

    Q_EMIT maximumHistoryChanged();
}

int HistoryProxySource::interval() const
{
    return m_updateTimer ? m_updateTimer->interval() : -1;
}

void HistoryProxySource::setInterval(int newInterval)
{
    if (m_updateTimer && newInterval == m_updateTimer->interval()) {
        return;
    }

    if (newInterval > 0) {
        if (!m_updateTimer) {
            m_updateTimer = std::make_unique<QTimer>();
            // We need precise timers to avoid missing updates when dealing with semi-constantly
            // updating source. That is, if the source updates at 500ms and we also update at that
            // rate, a drift of 2ms can cause us to miss updates.
            m_updateTimer->setTimerType(Qt::PreciseTimer);
            connect(m_updateTimer.get(), &QTimer::timeout, this, &HistoryProxySource::update);
        }
        m_updateTimer->setInterval(newInterval);
        m_updateTimer->start();
    } else {
        m_updateTimer.reset();
    }

    Q_EMIT intervalChanged();
}

HistoryProxySource::FillMode HistoryProxySource::fillMode() const
{
    return m_fillMode;
}

void HistoryProxySource::setFillMode(FillMode newFillMode)
{
    if (newFillMode == m_fillMode) {
        return;
    }

    m_fillMode = newFillMode;
    clear();
    Q_EMIT fillModeChanged();
}

void HistoryProxySource::clear()
{
    m_history.clear();
    Q_EMIT dataChanged();
}

void HistoryProxySource::update()
{
    if (!m_dataSource) {
        return;
    }

    m_history.prepend(m_dataSource->item(m_item));
    while (m_history.size() > 0 && m_history.size() > m_maximumHistory) {
        m_history.removeLast();
    }

    Q_EMIT dataChanged();
}

#include "moc_HistoryProxySource.cpp"
