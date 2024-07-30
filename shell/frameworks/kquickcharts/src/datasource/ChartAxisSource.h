/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef CHARTAXISSOURCE_H
#define CHARTAXISSOURCE_H

#include "ChartDataSource.h"

class XYChart;

/**
 * A data source that provides values from a chart's axis as data.
 */
class QUICKCHARTS_EXPORT ChartAxisSource : public ChartDataSource
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum class Axis { XAxis, YAxis };
    Q_ENUM(Axis)

    /**
     * Constructor
     *
     * @param parent TODO
     */
    ChartAxisSource(QObject *parent = nullptr);

    Q_PROPERTY(XYChart *chart READ chart WRITE setChart NOTIFY chartChanged)
    XYChart *chart() const;
    Q_SLOT void setChart(XYChart *newChart);
    Q_SIGNAL void chartChanged();

    Q_PROPERTY(ChartAxisSource::Axis axis READ axis WRITE setAxis NOTIFY axisChanged)
    ChartAxisSource::Axis axis() const;
    Q_SLOT void setAxis(ChartAxisSource::Axis newAxis);
    Q_SIGNAL void axisChanged();

    Q_PROPERTY(int itemCount READ itemCount WRITE setItemCount NOTIFY itemCountChanged)
    virtual int itemCount() const override;
    Q_SLOT void setItemCount(int newItemCount);
    Q_SIGNAL void itemCountChanged();

    virtual QVariant item(int index) const override;
    QVariant minimum() const override;
    QVariant maximum() const override;

private:
    XYChart *m_chart = nullptr;
    Axis m_axis = Axis::XAxis;
    int m_itemCount = 2;
};

#endif // CHARTAXISSOURCE_H
