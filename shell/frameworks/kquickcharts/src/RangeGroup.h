/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef RANGEGROUP_H
#define RANGEGROUP_H

#include <functional>

#include <QObject>
#include <qqmlregistration.h>

#include "quickcharts_export.h"

class ChartDataSource;

/**
 * An object that can be used as a grouped property to provide a value range for charts.
 *
 */
class QUICKCHARTS_EXPORT RangeGroup : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Range)
    QML_UNCREATABLE("Grouped Property")

public:
    struct RangeResult {
        qreal start = 0.0;
        qreal end = 0.0;
        qreal distance = 0.0;
    };

    explicit RangeGroup(QObject *parent = nullptr);

    /**
     * The start of this range.
     *
     * The default is 0.
     */
    Q_PROPERTY(qreal from READ from WRITE setFrom NOTIFY fromChanged)
    qreal from() const;
    void setFrom(qreal from);
    Q_SIGNAL void fromChanged();
    /**
     * The end of this range.
     *
     * The default is 100.
     */
    Q_PROPERTY(qreal to READ to WRITE setTo NOTIFY toChanged)
    qreal to() const;
    void setTo(qreal to);
    Q_SIGNAL void toChanged();
    /**
     * Whether to determine the range based on values of a chart.
     *
     * If true (the default), `from` and `to` are ignored and instead calculated
     * from the minimum and maximum values of a chart's valueSources.
     */
    Q_PROPERTY(bool automatic READ automatic WRITE setAutomatic NOTIFY automaticChanged)
    bool automatic() const;
    void setAutomatic(bool newAutomatic);
    Q_SIGNAL void automaticChanged();
    /**
     * The distance between from and to.
     */
    Q_PROPERTY(qreal distance READ distance NOTIFY rangeChanged)
    qreal distance() const;
    /**
     * The minimum size of the range.
     *
     * This is mostly relevant when automatic is true. Setting this value will
     * ensure that the range will never be smaller than this value. The default
     * is `std::numeric_limits<qreal>::min`, which means minimum is disabled.
     */
    Q_PROPERTY(qreal minimum READ minimum WRITE setMinimum NOTIFY minimumChanged)
    qreal minimum() const;
    void setMinimum(qreal newMinimum);
    Q_SIGNAL void minimumChanged();
    /**
     * The amount with which the range increases.
     *
     * The total range will be limited to a multiple of this value. This is
     * mostly useful when automatic is true. The default is 0.0, which means do
     * not limit the range increment.
     */
    Q_PROPERTY(qreal increment READ increment WRITE setIncrement NOTIFY incrementChanged)
    qreal increment() const;
    void setIncrement(qreal newIncrement);
    Q_SIGNAL void incrementChanged();

    bool isValid() const;

    Q_SIGNAL void rangeChanged();

    RangeResult calculateRange(const QList<ChartDataSource *> &sources,
                               std::function<qreal(ChartDataSource *)> minimumCallback,
                               std::function<qreal(ChartDataSource *)> maximumCallback);

private:
    qreal m_from = 0.0;
    qreal m_to = 100.0;
    bool m_automatic = true;
    qreal m_minimum = std::numeric_limits<qreal>::min();
    qreal m_increment = 0.0;
};

#endif // RANGEGROUP_H
