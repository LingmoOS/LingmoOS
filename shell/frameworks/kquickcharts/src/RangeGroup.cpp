/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "RangeGroup.h"

#include <cmath>

#include <QList>

RangeGroup::RangeGroup(QObject *parent)
    : QObject(parent)
{
    connect(this, &RangeGroup::fromChanged, this, &RangeGroup::rangeChanged);
    connect(this, &RangeGroup::toChanged, this, &RangeGroup::rangeChanged);
    connect(this, &RangeGroup::automaticChanged, this, &RangeGroup::rangeChanged);
    connect(this, &RangeGroup::minimumChanged, this, &RangeGroup::rangeChanged);
    connect(this, &RangeGroup::incrementChanged, this, &RangeGroup::rangeChanged);
}

qreal RangeGroup::from() const
{
    return m_from;
}

void RangeGroup::setFrom(qreal from)
{
    if (qFuzzyCompare(m_from, from)) {
        return;
    }

    m_from = from;
    Q_EMIT fromChanged();
}

qreal RangeGroup::to() const
{
    return m_to;
}

void RangeGroup::setTo(qreal to)
{
    if (qFuzzyCompare(m_to, to)) {
        return;
    }

    m_to = to;
    Q_EMIT toChanged();
}

bool RangeGroup::automatic() const
{
    return m_automatic;
}

void RangeGroup::setAutomatic(bool automatic)
{
    if (m_automatic == automatic) {
        return;
    }

    m_automatic = automatic;
    Q_EMIT automaticChanged();
}

qreal RangeGroup::distance() const
{
    return m_to - m_from;
}

qreal RangeGroup::minimum() const
{
    return m_minimum;
}

void RangeGroup::setMinimum(qreal newMinimum)
{
    if (newMinimum == m_minimum) {
        return;
    }

    m_minimum = newMinimum;
    Q_EMIT minimumChanged();
}

qreal RangeGroup::increment() const
{
    return m_increment;
}

void RangeGroup::setIncrement(qreal newIncrement)
{
    if (newIncrement == m_increment) {
        return;
    }

    m_increment = newIncrement;
    Q_EMIT incrementChanged();
}

bool RangeGroup::isValid() const
{
    return m_automatic || (m_to > m_from);
}

RangeGroup::RangeResult RangeGroup::calculateRange(const QList<ChartDataSource *> &sources,
                                                   std::function<qreal(ChartDataSource *)> minimumCallback,
                                                   std::function<qreal(ChartDataSource *)> maximumCallback)
{
    RangeResult result;

    auto min = std::numeric_limits<qreal>::max();
    auto max = std::numeric_limits<qreal>::min();

    if (!m_automatic) {
        min = m_from;
        max = m_to;
    } else {
        std::for_each(sources.begin(), sources.end(), [&](ChartDataSource *source) {
            min = std::min(min, minimumCallback(source));
            max = std::max(max, maximumCallback(source));
        });
    }

    max = std::max(max, m_minimum);
    if (m_increment > 0.0) {
        max = m_increment * std::ceil(max / m_increment);
    }

    result.start = min;
    result.end = max;
    result.distance = max - min;

    return result;
}

#include "moc_RangeGroup.cpp"
