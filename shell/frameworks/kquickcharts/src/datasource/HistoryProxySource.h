/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef HISTORYPROXYSOURCE_H
#define HISTORYPROXYSOURCE_H

#include <QList>
#include <QTimer>
#include <QVariant>
#include <memory>

#include "ChartDataSource.h"

/**
 * A data source that provides a history of a single item of a different data source.
 *
 * This data source will monitor a single item of another data source for changes
 * and record them, exposing historical values as
 */
class QUICKCHARTS_EXPORT HistoryProxySource : public ChartDataSource
{
    Q_OBJECT
    QML_ELEMENT

public:
    /**
     * The different fill modes.
     *
     * These determine the value returned by \ref itemCount and what happens
     * when requesting an item that does not have a value in the history.
     */
    enum FillMode {
        /**
         * Do not fill with any items. The source's \ref itemCount will be equal
         * to the number of items in the history.
         */
        DoNotFill,
        /**
         * Fill with empty values, starting at 0. The source's \ref itemCount
         * will be equal to \ref maximumHistory. Items that do not have a value
         * in the history will return a default-constructed value based on the
         * first item of the source.
         */
        FillFromStart,
        /**
         * Fill with empty values, placing partial history at the end. This
         * means that the first recorded history item will be shown at position
         * `maximumHistory - 1`, the second at `maximumHistory - 2` and so on,
         * until \ref maximumHistory is reached, after which items will be
         * discarded normally.
         */
        FillFromEnd
    };
    Q_ENUM(FillMode)

    explicit HistoryProxySource(QObject *parent = nullptr);

    /**
     * The data source to read data from.
     *
     * This will use the item at \ref item from the provided source and use that
     * to fill the history of this source.
     *
     * \note Changing this property will clear the existing history.
     */
    Q_PROPERTY(ChartDataSource *source READ source WRITE setSource NOTIFY sourceChanged)
    ChartDataSource *source() const;
    void setSource(ChartDataSource *newSource);
    Q_SIGNAL void sourceChanged();
    /**
     * The item of the data source to read data from.
     *
     * This item will be read either when the source's dataChanged has been
     * emitted or on an interval if \ref interval has been set.
     *
     * The default is 0.
     *
     * \note Changing this property will clear the existing history.
     */
    Q_PROPERTY(int item READ item WRITE setItem NOTIFY itemChanged)
    int item() const;
    void setItem(int newItem);
    Q_SIGNAL void itemChanged();
    /**
     * The maximum amount of history to keep.
     *
     * The default is 10.
     */
    Q_PROPERTY(int maximumHistory READ maximumHistory WRITE setMaximumHistory NOTIFY maximumHistoryChanged)
    int maximumHistory() const;
    void setMaximumHistory(int maximumHistory);
    Q_SIGNAL void maximumHistoryChanged();
    /**
     * The interval, in milliseconds, with which to query the data source.
     *
     * If set to a value <= 0, a new item will be added whenever \ref source
     * changes. Otherwise, source will be sampled every interval milliseconds
     * and a new item will be added with whatever value it has at that point,
     * even if it did not change.
     *
     * The default is 0.
     */
    Q_PROPERTY(int interval READ interval WRITE setInterval NOTIFY intervalChanged)
    int interval() const;
    void setInterval(int newInterval);
    Q_SIGNAL void intervalChanged();
    /**
     * The fill mode.
     *
     * This determines what happens when there is not enough history yet. See
     * \ref FillMode for which modes are available.
     *
     * The default is DoNotFill.
     *
     * \note Changing this property will clear the existing history.
     */
    Q_PROPERTY(FillMode fillMode READ fillMode WRITE setFillMode NOTIFY fillModeChanged)
    FillMode fillMode() const;
    void setFillMode(FillMode newFillMode);
    Q_SIGNAL void fillModeChanged();

    /**
     * Clear the entire history of this source.
     */
    Q_INVOKABLE void clear();

    int itemCount() const override;
    QVariant item(int index) const override;
    QVariant minimum() const override;
    QVariant maximum() const override;
    QVariant first() const override;

private:
    void update();

    ChartDataSource *m_dataSource = nullptr;
    int m_item = 0;
    int m_maximumHistory = 10;
    FillMode m_fillMode = DoNotFill;
    std::unique_ptr<QTimer> m_updateTimer;
    QList<QVariant> m_history;
};

#endif // HISTORYPROXYSOURCE_H
