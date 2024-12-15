// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "grandsearchlistmodel.h"

#include <QDebug>

using namespace GrandSearch;

GrandSearchListModel::GrandSearchListModel(int rows, int columns, QObject *parent):
    QStandardItemModel(rows, columns, parent)
{

}

GrandSearchListModel::~GrandSearchListModel()
{

}

bool GrandSearchListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QStandardItemModel::setData(index, value, role);
}

Qt::ItemFlags GrandSearchListModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (index.isValid()) {
        return Qt::ItemIsDragEnabled | defaultFlags;
    } else {
        return Qt::ItemIsDropEnabled | defaultFlags;
    }
}
