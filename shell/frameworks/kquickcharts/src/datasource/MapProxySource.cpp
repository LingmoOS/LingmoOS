/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "MapProxySource.h"

MapProxySource::MapProxySource(QObject *parent)
    : ChartDataSource(parent)
{
    connect(this, &MapProxySource::sourceChanged, this, &ChartDataSource::dataChanged);
    connect(this, &MapProxySource::mapChanged, this, &ChartDataSource::dataChanged);
}

int MapProxySource::itemCount() const
{
    if (m_source) {
        return m_source->itemCount();
    }

    return 0;
}

QVariant MapProxySource::minimum() const
{
    auto itr = std::min_element(m_map.cbegin(), m_map.cend(), variantCompare);
    if (itr != m_map.cend()) {
        return *itr;
    }
    return QVariant{};
}

QVariant MapProxySource::maximum() const
{
    auto itr = std::max_element(m_map.cbegin(), m_map.cend(), variantCompare);
    if (itr != m_map.cend()) {
        return *itr;
    }
    return QVariant{};
}

QVariant MapProxySource::item(int index) const
{
    if (!m_source) {
        return QVariant{};
    }

    auto mapIndex = m_source->item(index).toString();
    if (mapIndex.isEmpty()) {
        return QVariant{};
    }

    return m_map.value(mapIndex);
}

ChartDataSource *MapProxySource::source() const
{
    return m_source;
}

void MapProxySource::setSource(ChartDataSource *newSource)
{
    if (newSource == m_source) {
        return;
    }

    if (m_source) {
        m_source->disconnect(this);
    }

    m_source = newSource;
    if (m_source) {
        connect(m_source, &ChartDataSource::dataChanged, this, &ChartDataSource::dataChanged);
    }
    Q_EMIT sourceChanged();
}

QVariantMap MapProxySource::map() const
{
    return m_map;
}

void MapProxySource::setMap(const QVariantMap &newMap)
{
    if (newMap == m_map) {
        return;
    }

    m_map = newMap;

    Q_EMIT mapChanged();
}

#include "moc_MapProxySource.cpp"
