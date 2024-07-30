/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef ARRAYSOURCE_H
#define ARRAYSOURCE_H

#include <QVariantList>

#include "ChartDataSource.h"

/**
 * A data source that provides entries of an array as data.
 */
class QUICKCHARTS_EXPORT ArraySource : public ChartDataSource
{
    Q_OBJECT
    QML_ELEMENT

public:
    /**
     * Constructor
     *
     * @param parent TODO
     */
    explicit ArraySource(QObject *parent = nullptr);

    virtual int itemCount() const override;
    virtual QVariant item(int index) const override;
    QVariant minimum() const override;
    QVariant maximum() const override;

    Q_PROPERTY(QVariantList array READ array WRITE setArray NOTIFY dataChanged)
    QVariantList array() const;
    void setArray(const QVariantList &array);

    Q_PROPERTY(bool wrap READ wrap WRITE setWrap NOTIFY dataChanged)
    bool wrap() const;
    void setWrap(bool wrap);

private:
    QVariantList m_array;
    bool m_wrap = false;
};

#endif // ARRAYSOURCE_H
