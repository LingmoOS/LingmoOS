/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "LegendModel.h"

#include "Chart.h"
#include "datasource/ChartDataSource.h"

LegendModel::LegendModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QHash<int, QByteArray> LegendModel::roleNames() const
{
    static QHash<int, QByteArray> names = {
        {NameRole, "name"},
        {ShortNameRole, "shortName"},
        {ColorRole, "color"},
        {ValueRole, "value"},
    };

    return names;
}

int LegendModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_items.size();
}

QVariant LegendModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::ParentIsInvalid | CheckIndexOption::IndexIsValid)) {
        return {};
    }

    switch (role) {
    case NameRole:
        return m_items.at(index.row()).name;
    case ShortNameRole:
        return m_items.at(index.row()).shortName;
    case ColorRole:
        return m_items.at(index.row()).color;
    case ValueRole:
        return m_items.at(index.row()).value;
    }

    return QVariant{};
}

Chart *LegendModel::chart() const
{
    return m_chart;
}

void LegendModel::setChart(Chart *newChart)
{
    if (newChart == m_chart) {
        return;
    }

    if (m_chart) {
        for (const auto &connection : std::as_const(m_connections)) {
            disconnect(connection);
        }
        m_connections.clear();
    }

    m_chart = newChart;
    queueUpdate();
    Q_EMIT chartChanged();
}

int LegendModel::sourceIndex() const
{
    return m_sourceIndex;
}

void LegendModel::setSourceIndex(int index)
{
    if (index == m_sourceIndex) {
        return;
    }

    m_sourceIndex = index;
    queueUpdate();
    Q_EMIT sourceIndexChanged();
}

void LegendModel::queueUpdate()
{
    if (!m_updateQueued) {
        m_updateQueued = true;
        QMetaObject::invokeMethod(this, &LegendModel::update, Qt::QueuedConnection);
    }
}

void LegendModel::queueDataChange()
{
    if (!m_dataChangeQueued) {
        m_dataChangeQueued = true;
        QMetaObject::invokeMethod(this, &LegendModel::updateData, Qt::QueuedConnection);
    }
}

void LegendModel::update()
{
    m_updateQueued = false;

    if (!m_chart) {
        return;
    }

    beginResetModel();
    m_items.clear();

    ChartDataSource *colorSource = m_chart->colorSource();
    ChartDataSource *nameSource = m_chart->nameSource();
    ChartDataSource *shortNameSource = m_chart->shortNameSource();

    m_connections.push_back(connect(m_chart, &Chart::colorSourceChanged, this, &LegendModel::queueUpdate, Qt::UniqueConnection));
    m_connections.push_back(connect(m_chart, &Chart::nameSourceChanged, this, &LegendModel::queueUpdate, Qt::UniqueConnection));

    auto sources = m_chart->valueSources();
    int itemCount = countItems();

    std::transform(sources.cbegin(), sources.cend(), std::back_inserter(m_connections), [this](ChartDataSource *source) {
        return connect(source, &ChartDataSource::dataChanged, this, &LegendModel::queueDataChange, Qt::UniqueConnection);
    });

    m_connections.push_back(connect(m_chart, &Chart::valueSourcesChanged, this, &LegendModel::queueUpdate, Qt::UniqueConnection));

    if ((!colorSource && !(nameSource || shortNameSource)) || itemCount <= 0) {
        endResetModel();
        return;
    }

    if (colorSource) {
        m_connections.push_back(connect(colorSource, &ChartDataSource::dataChanged, this, &LegendModel::queueDataChange, Qt::UniqueConnection));
    }

    if (nameSource) {
        m_connections.push_back(connect(nameSource, &ChartDataSource::dataChanged, this, &LegendModel::queueDataChange, Qt::UniqueConnection));
    }

    if (shortNameSource) {
        m_connections.push_back(connect(shortNameSource, &ChartDataSource::dataChanged, this, &LegendModel::queueDataChange, Qt::UniqueConnection));
    }

    for (int i = 0; i < itemCount; ++i) {
        LegendItem item;
        item.name = nameSource ? nameSource->item(i).toString() : QString();
        item.shortName = shortNameSource ? shortNameSource->item(i).toString() : QString();
        item.color = colorSource ? colorSource->item(i).value<QColor>() : QColor();
        item.value = getValueForItem(i);
        m_items.push_back(item);
    }

    endResetModel();
}

void LegendModel::updateData()
{
    ChartDataSource *colorSource = m_chart->colorSource();
    ChartDataSource *nameSource = m_chart->nameSource();
    ChartDataSource *shortNameSource = m_chart->shortNameSource();

    auto itemCount = countItems();

    m_dataChangeQueued = false;

    if (itemCount != int(m_items.size())) {
        // Number of items changed, so trigger a full update
        queueUpdate();
        return;
    }

    QList<QList<int>> changedRows(itemCount);

    std::for_each(m_items.begin(), m_items.end(), [&, i = 0](LegendItem &item) mutable {
        auto name = nameSource ? nameSource->item(i).toString() : QString{};
        if (item.name != name) {
            item.name = name;
            changedRows[i] << NameRole;
        }

        auto shortName = shortNameSource ? shortNameSource->item(i).toString() : QString{};
        if (item.shortName != shortName) {
            item.shortName = shortName;
            changedRows[i] << ShortNameRole;
        }

        auto color = colorSource ? colorSource->item(i).toString() : QColor{};
        if (item.color != color) {
            item.color = color;
            changedRows[i] << ColorRole;
        }

        auto value = getValueForItem(i);
        if (item.value != value) {
            item.value = value;
            changedRows[i] << ValueRole;
        }

        i++;
    });

    for (auto i = 0; i < changedRows.size(); ++i) {
        auto changedRoles = changedRows.at(i);
        if (!changedRoles.isEmpty()) {
            Q_EMIT dataChanged(index(i, 0), index(i, 0), changedRoles);
        }
    }
}

int LegendModel::countItems()
{
    auto sources = m_chart->valueSources();
    int itemCount = 0;

    switch (m_chart->indexingMode()) {
    case Chart::IndexSourceValues:
        if (sources.count() > 0) {
            itemCount = sources.at(0)->itemCount();
        }
        break;
    case Chart::IndexEachSource:
        itemCount = sources.count();
        break;
    case Chart::IndexAllValues:
        itemCount = std::accumulate(sources.cbegin(), sources.cend(), 0, [](int current, ChartDataSource *source) -> int {
            return current + source->itemCount();
        });
        break;
    }

    return itemCount;
}

QVariant LegendModel::getValueForItem(int item)
{
    const auto sources = m_chart->valueSources();
    auto value = QVariant{};

    switch (m_chart->indexingMode()) {
    case Chart::IndexSourceValues:
        value = sources.at(0)->item(item);
        break;
    case Chart::IndexEachSource:
        value = sources.at(item)->first();
        break;
    case Chart::IndexAllValues:
        for (auto source : sources) {
            if (item >= source->itemCount()) {
                item -= source->itemCount();
            } else {
                value = source->item(item);
                break;
            }
        }
        break;
    }

    return value;
}

#include "moc_LegendModel.cpp"
