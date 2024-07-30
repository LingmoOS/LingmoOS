/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "ArraySource.h"

ArraySource::ArraySource(QObject *parent)
    : ChartDataSource(parent)
{
}

int ArraySource::itemCount() const
{
    return m_array.count();
}

QVariant ArraySource::item(int index) const
{
    if (m_array.isEmpty()) {
        return {};
    }

    if (!m_wrap && (index < 0 || index > m_array.count() - 1)) {
        return {};
    }

    return m_array.at(index % m_array.count());
}

QVariant ArraySource::minimum() const
{
    auto itr = std::min_element(m_array.cbegin(), m_array.cend(), variantCompare);
    if (itr != m_array.cend()) {
        return *itr;
    }
    return QVariant{};
}

QVariant ArraySource::maximum() const
{
    auto itr = std::max_element(m_array.cbegin(), m_array.cend(), variantCompare);
    if (itr != m_array.cend()) {
        return *itr;
    }
    return QVariant{};
}

QVariantList ArraySource::array() const
{
    return m_array;
}

bool ArraySource::wrap() const
{
    return m_wrap;
}

void ArraySource::setArray(const QVariantList &array)
{
    if (m_array == array) {
        return;
    }

    m_array = array;
    Q_EMIT dataChanged();
}

void ArraySource::setWrap(bool wrap)
{
    if (m_wrap == wrap) {
        return;
    }

    m_wrap = wrap;
    Q_EMIT dataChanged();
}

#include "moc_ArraySource.cpp"
