// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETIF_INFO_SORT_FILTER_PROXY_MODEL_H
#define NETIF_INFO_SORT_FILTER_PROXY_MODEL_H

#include <QSortFilterProxyModel>

//class NetifInfoSortFilterProxyModel : public QSortFilterProxyModel
//{
//    Q_OBJECT

//public:
//    enum SortKey {
//        kSortByName = 0,
//        kSortByIndex,
//        kSortBySentSpeed,
//        kSortByRecvSpeed,
//        kSortByTotalSentBytes,
//        kSortByTotalRecvBytes,
//        kSortByTotalSentPackets,
//        kSortByTotalRecvPackets,

//        kSortKeyMax
//    };

//    explicit NetifInfoSortFilterProxyModel(QObject *parent = nullptr);
//    virtual ~NetifInfoSortFilterProxyModel() override;

//    void setSortKey(enum SortKey key = kSortByName);

//protected:
//    bool filterAcceptsRow(int row, const QModelIndex &parent) const override;
//    bool filterAcceptsColumn(int column, const QModelIndex &parent) const override;
//    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

//private:
//    enum SortKey m_sortKey;
//};

//inline void NetifInfoSortFilterProxyModel::setSortKey(NetifInfoSortFilterProxyModel::SortKey key)
//{
//    m_sortKey = key;
//}

#endif // NETIF_INFO_SORT_FILTER_PROXY_MODEL_H
