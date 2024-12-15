// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSTEM_SERVICE_SORT_FILTER_PROXY_MODEL_H
#define SYSTEM_SERVICE_SORT_FILTER_PROXY_MODEL_H

#include <QSortFilterProxyModel>

/**
 * @brief Sort filter proxy model for service model
 */
class SystemServiceSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    /**
     * @brief Proxy model constructor
     * @param parent Parent object
     */
    explicit SystemServiceSortFilterProxyModel(QObject *parent = nullptr);
    /**
     * @brief Check if more data can be fetched for given parent
     * @param parent
     * @return Returns true if there is more data available for parent; otherwise returns false
     */
    bool canFetchMore(const QModelIndex &parent) const override;
    /**
     * @brief Fetches any available data for the items with the parent specified by the parent index
     * @param parent Parent index
     */
    void fetchMore(const QModelIndex &parent) override;

protected:
    /**
     * @brief Filters the row of specified parent with given pattern
     * @param row The row to be filtered
     * @param parent Parent index of the row
     * @return Returns true if the item in the row indicated by the given row and parent should be included in the model; otherwise returns false
     */
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override;
    /**
     * @brief Filters the column of specified parent with given pattern
     * @param column The column to be filtered
     * @param parent Parent index of the row
     * @return Returns true if the item in the column indicated by the given column and parent should be included in the model; otherwise returns false
     */
    bool filterAcceptsColumn(int column, const QModelIndex &parent) const override;
    /**
     * @brief Compare two items with the specified index
     * @param left Index of the item to be compared on left side
     * @param right Index of the item to be compared on right side
     * @return Returns true if the value of the item referred to by the given index left is less than the value of the item referred to by the given index right, otherwise returns false
     */
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
};

#endif  // SYSTEM_SERVICE_SORT_FILTER_PROXY_MODEL_H
