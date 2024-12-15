// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netif_info_sort_filter_proxy_model.h"

#include "netif_info_model.h"

//NetifInfoSortFilterProxyModel::NetifInfoSortFilterProxyModel(QObject *parent)
//    : QSortFilterProxyModel(parent)
//{
//}

//NetifInfoSortFilterProxyModel::~NetifInfoSortFilterProxyModel()
//{
//}

//bool NetifInfoSortFilterProxyModel::filterAcceptsRow(int row, const QModelIndex &parent) const
//{
//    QModelIndex name = sourceModel()->index(row, NetifInfoModel::kPropDevName, parent);
//    QModelIndex brand = sourceModel()->index(row, NetifInfoModel::kPropBrand, parent);
//    QModelIndex essid = sourceModel()->index(row, NetifInfoModel::kPropESSID, parent);
//    // TODO: add connection type matches

//    bool ok = false;
//    if (name.isValid()) {
//        ok |= sourceModel()->data(name).toString().contains(filterRegExp());
//    }
//    if (brand.isValid()) {
//        ok |= sourceModel()->data(brand).toString().contains(filterRegExp());
//    }
//    if (essid.isValid()) {
//        ok |= sourceModel()->data(essid).toString().contains(filterRegExp());
//    }

//    return ok;
//}

//bool NetifInfoSortFilterProxyModel::filterAcceptsColumn(int column, const QModelIndex &parent) const
//{
//    return QSortFilterProxyModel::filterAcceptsColumn(column, parent);
//}

//bool NetifInfoSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
//{
//    QModelIndex idx {};

//    switch (m_sortKey) {
//    case kSortByName: {
//        QString lhs {}, rhs {};

//        idx = sourceModel()->index(left.row(), NetifInfoModel::kPropDevName);
//        if (idx.isValid())
//            lhs = idx.data().toString();

//        idx = sourceModel()->index(right.row(), NetifInfoModel::kPropDevName);
//        if (idx.isValid())
//            rhs = idx.data().toString();

//        return lhs.localeAwareCompare(rhs) < 0;
//    }
//    case kSortByIndex: {
//        int lhs {}, rhs {};

//        idx = sourceModel()->index(left.row(), NetifInfoModel::kPropIndex);
//        if (idx.isValid())
//            lhs = idx.data().toInt();

//        idx = sourceModel()->index(right.row(), NetifInfoModel::kPropIndex);
//        if (idx.isValid())
//            rhs = idx.data().toInt();

//        return lhs < rhs;
//    }
//    case kSortBySentSpeed: {
//        qreal lhs {}, rhs {};

//        idx = sourceModel()->index(left.row(), NetifInfoModel::kPropRecvBps);
//        if (idx.isValid())
//            lhs = idx.data(NetifInfoModel::kValueRole).toReal();

//        idx = sourceModel()->index(right.row(), NetifInfoModel::kPropRecvBps);
//        if (idx.isValid())
//            rhs = idx.data(NetifInfoModel::kValueRole).toReal();

//        return lhs < rhs;
//    }
//    case kSortByRecvSpeed: {
//        qreal lhs {}, rhs {};

//        idx = sourceModel()->index(left.row(), NetifInfoModel::kPropSentBps);
//        if (idx.isValid())
//            lhs = idx.data(NetifInfoModel::kValueRole).toReal();

//        idx = sourceModel()->index(right.row(), NetifInfoModel::kPropSentBps);
//        if (idx.isValid())
//            rhs = idx.data(NetifInfoModel::kValueRole).toReal();

//        return lhs < rhs;
//    }
//    case kSortByTotalSentBytes: {
//        qulonglong lhs {}, rhs {};

//        idx = sourceModel()->index(left.row(), NetifInfoModel::kPropRxBytes);
//        if (idx.isValid())
//            lhs = idx.data(NetifInfoModel::kValueRole).toULongLong();

//        idx = sourceModel()->index(right.row(), NetifInfoModel::kPropRxBytes);
//        if (idx.isValid())
//            rhs = idx.data(NetifInfoModel::kValueRole).toULongLong();

//        return lhs < rhs;
//    }
//    case kSortByTotalRecvBytes: {
//        qulonglong lhs {}, rhs {};

//        idx = sourceModel()->index(left.row(), NetifInfoModel::kPropTxBytes);
//        if (idx.isValid())
//            lhs = idx.data(NetifInfoModel::kValueRole).toULongLong();

//        idx = sourceModel()->index(right.row(), NetifInfoModel::kPropTxBytes);
//        if (idx.isValid())
//            rhs = idx.data(NetifInfoModel::kValueRole).toULongLong();

//        return lhs < rhs;
//    }
//    case kSortByTotalSentPackets: {
//        qulonglong lhs {}, rhs {};

//        idx = sourceModel()->index(left.row(), NetifInfoModel::kPropRxPackets);
//        if (idx.isValid())
//            lhs = idx.data().toULongLong();

//        idx = sourceModel()->index(right.row(), NetifInfoModel::kPropRxPackets);
//        if (idx.isValid())
//            rhs = idx.data().toULongLong();

//        return lhs < rhs;
//    }
//    case kSortByTotalRecvPackets: {
//        qulonglong lhs {}, rhs {};

//        idx = sourceModel()->index(left.row(), NetifInfoModel::kPropTxPackets);
//        if (idx.isValid())
//            lhs = idx.data().toULongLong();

//        idx = sourceModel()->index(right.row(), NetifInfoModel::kPropTxPackets);
//        if (idx.isValid())
//            rhs = idx.data().toULongLong();

//        return lhs < rhs;
//    }
//    default:
//        break;
//    } // ::switch

//    return QSortFilterProxyModel::lessThan(left, right);
//}
