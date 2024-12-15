// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRANDSEARCHLISTMODEL_H
#define GRANDSEARCHLISTMODEL_H

#include <QStandardItemModel>
#include <QScopedPointer>

namespace GrandSearch {

class GrandSearchListModel : public QStandardItemModel
{
    Q_OBJECT
public:
    GrandSearchListModel(int rows, int columns, QObject *parent = Q_NULLPTR);
    ~GrandSearchListModel() override;

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
};

}

#endif // GRANDSEARCHLISTMODEL_H
