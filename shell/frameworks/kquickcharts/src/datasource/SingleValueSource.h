/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef SINGLEVALUESOURCE_H
#define SINGLEVALUESOURCE_H

#include <QVariant>

#include "ChartDataSource.h"

/**
 * A data source that provides a single value as data.
 */
class QUICKCHARTS_EXPORT SingleValueSource : public ChartDataSource
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit SingleValueSource(QObject *parent = nullptr);

    virtual int itemCount() const override;
    virtual QVariant item(int index) const override;
    QVariant minimum() const override;
    QVariant maximum() const override;

    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY dataChanged)
    QVariant value() const;
    void setValue(const QVariant &value);

private:
    QVariant m_value;
};

#endif // SINGLEVALUESOURCE_H
