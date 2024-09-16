// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLOCK_DEV_INFO_SORT_FILTER_PROXY_MODEL_H
#define BLOCK_DEV_INFO_SORT_FILTER_PROXY_MODEL_H

//#include <QSortFilterProxyModel>

//class BlockDevInfoModel;

//class BlockDevInfoSortFilterProxyModel : public QSortFilterProxyModel
//{
//    Q_OBJECT

//public:
//    enum SortKey {
//        kSortByName = 0,
//        kSortByCapacity,
//        kSortByTotalRead,
//        kSortByTotalWrite,
//        kSortByReadSpeed,
//        kSortByWriteSpeed,
//        kSortByTPS,
//        kSortByModel,

//        kSortKeyMax
//    };

//    explicit BlockDevInfoSortFilterProxyModel(QObject *parent = nullptr);
//    virtual ~BlockDevInfoSortFilterProxyModel() override;

//    void setSortKey(enum SortKey key = kSortByName);

//protected:
//    bool filterAcceptsRow(int row, const QModelIndex &parent) const override;
//    bool filterAcceptsColumn(int column, const QModelIndex &parent) const override;
//    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

//private:
//    enum SortKey m_sortKey;
//};

//inline void BlockDevInfoSortFilterProxyModel::setSortKey(BlockDevInfoSortFilterProxyModel::SortKey key)
//{
//    m_sortKey = key;
//}

#endif // BLOCK_DEV_INFO_SORT_FILTER_PROXY_MODEL_H
