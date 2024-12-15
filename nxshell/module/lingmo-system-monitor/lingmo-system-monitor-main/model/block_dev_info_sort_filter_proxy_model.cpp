// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//#include "block_dev_info_sort_filter_proxy_model.h"

//#include "block_dev_info_model.h"

//#include <QString>

//BlockDevInfoSortFilterProxyModel::BlockDevInfoSortFilterProxyModel(QObject *parent)
//    : QSortFilterProxyModel(parent)
//    , m_sortKey(kSortByName)
//{
//}

//BlockDevInfoSortFilterProxyModel::~BlockDevInfoSortFilterProxyModel()
//{
//}

//bool BlockDevInfoSortFilterProxyModel::filterAcceptsRow(int row, const QModelIndex &parent) const
//{
//    QModelIndex name = sourceModel()->index(row, BlockDevInfoModel::kPropDevName, parent);
//    QModelIndex model = sourceModel()->index(row, BlockDevInfoModel::kPropModel, parent);

//    bool ok = false;
//    if (name.isValid()) {
//        ok |= sourceModel()->data(name).toString().contains(filterRegExp());
//    }
//    if (model.isValid()) {
//        ok |= sourceModel()->data(model).toString().contains(filterRegExp());
//    }

//    return ok;
//}

//bool BlockDevInfoSortFilterProxyModel::filterAcceptsColumn(int column, const QModelIndex &parent) const
//{
//    return QSortFilterProxyModel::filterAcceptsColumn(column, parent);
//}

//bool BlockDevInfoSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
//{
//    QModelIndex idx {};
//    switch (m_sortKey) {
//    case kSortByName: {
//        QString lhs {}, rhs {};

//        idx = sourceModel()->index(left.row(), BlockDevInfoModel::kPropDevName);
//        if (idx.isValid())
//            lhs = idx.data().toString();

//        idx = sourceModel()->index(right.row(), BlockDevInfoModel::kPropDevName);
//        if (idx.isValid())
//            rhs = idx.data().toString();

//        return lhs.localeAwareCompare(rhs) < 0;
//    }
//    case kSortByModel: {
//        QString lhs {}, rhs {};

//        idx = sourceModel()->index(left.row(), BlockDevInfoModel::kPropModel);
//        if (idx.isValid())
//            lhs = idx.data().toString();

//        idx = sourceModel()->index(right.row(), BlockDevInfoModel::kPropModel);
//        if (idx.isValid())
//            rhs = idx.data().toString();

//        return lhs.localeAwareCompare(rhs) < 0;
//    }
//    case kSortByCapacity: {
//        qulonglong lhs {}, rhs {};

//        idx = sourceModel()->index(left.row(), BlockDevInfoModel::kPropCapacity);
//        if (idx.isValid())
//            lhs = idx.data(BlockDevInfoModel::kValueRole).toULongLong();

//        idx = sourceModel()->index(right.row(), BlockDevInfoModel::kPropCapacity);
//        if (idx.isValid())
//            rhs = idx.data(BlockDevInfoModel::kValueRole).toULongLong();

//        return lhs < rhs;
//    }
//    case kSortByTotalRead: {
//        qulonglong lhs {}, rhs {};

//        idx = sourceModel()->index(left.row(), BlockDevInfoModel::kPropTotalRead);
//        if (idx.isValid())
//            lhs = idx.data(BlockDevInfoModel::kValueRole).toULongLong();

//        idx = sourceModel()->index(right.row(), BlockDevInfoModel::kPropTotalRead);
//        if (idx.isValid())
//            rhs = idx.data(BlockDevInfoModel::kValueRole).toULongLong();

//        return lhs < rhs;
//    }
//    case kSortByTotalWrite: {
//        qulonglong lhs {}, rhs {};

//        idx = sourceModel()->index(left.row(), BlockDevInfoModel::kPropTotalWrtn);
//        if (idx.isValid())
//            lhs = idx.data(BlockDevInfoModel::kValueRole).toULongLong();

//        idx = sourceModel()->index(right.row(), BlockDevInfoModel::kPropTotalWrtn);
//        if (idx.isValid())
//            rhs = idx.data(BlockDevInfoModel::kValueRole).toULongLong();

//        return lhs < rhs;
//    }
//    case kSortByReadSpeed: {
//        qreal lhs {}, rhs {};

//        idx = sourceModel()->index(left.row(), BlockDevInfoModel::kPropReadBps);
//        if (idx.isValid())
//            lhs = idx.data(BlockDevInfoModel::kValueRole).toReal();

//        idx = sourceModel()->index(right.row(), BlockDevInfoModel::kPropReadBps);
//        if (idx.isValid())
//            rhs = idx.data(BlockDevInfoModel::kValueRole).toReal();

//        return lhs < rhs;
//    }
//    case kSortByWriteSpeed: {
//        qreal lhs {}, rhs {};

//        idx = sourceModel()->index(left.row(), BlockDevInfoModel::kPropWriteBps);
//        if (idx.isValid())
//            lhs = idx.data(BlockDevInfoModel::kValueRole).toReal();

//        idx = sourceModel()->index(right.row(), BlockDevInfoModel::kPropWriteBps);
//        if (idx.isValid())
//            rhs = idx.data(BlockDevInfoModel::kValueRole).toReal();

//        return lhs < rhs;
//    }
//    case kSortByTPS: {
//        qulonglong lhs {}, rhs {};

//        idx = sourceModel()->index(left.row(), BlockDevInfoModel::kPropTps);
//        if (idx.isValid())
//            lhs = idx.data().toULongLong();

//        idx = sourceModel()->index(right.row(), BlockDevInfoModel::kPropTps);
//        if (idx.isValid())
//            rhs = idx.data().toULongLong();

//        return lhs < rhs;
//    }
//    default:
//        break;
//    }

//    return QSortFilterProxyModel::lessThan(left, right);
//}
