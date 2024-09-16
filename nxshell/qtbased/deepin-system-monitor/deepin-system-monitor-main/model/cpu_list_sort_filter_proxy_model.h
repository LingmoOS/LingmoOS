// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CPU_LIST_SORT_FILTER_PROXY_MODEL_H
#define CPU_LIST_SORT_FILTER_PROXY_MODEL_H

#include <QSortFilterProxyModel>

class CPUListSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    enum SortKey {
        kSortByIndex = 0,
        kSortByUsage,

        kSortKeyMax
    };

    explicit CPUListSortFilterProxyModel(QObject *parent = nullptr);
    virtual ~CPUListSortFilterProxyModel() override = default;

    void setSortKey(enum SortKey key = kSortByIndex);

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override;
    bool filterAcceptsColumn(int column, const QModelIndex &parent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    enum SortKey m_sortKey;
};

inline void CPUListSortFilterProxyModel::setSortKey(CPUListSortFilterProxyModel::SortKey key)
{
    m_sortKey = key;
}

#endif // CPU_LIST_SORT_FILTER_PROXY_MODEL_H
