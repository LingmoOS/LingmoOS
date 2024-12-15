// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SORTPROXY_H
#define SORTPROXY_H

#include <QSortFilterProxyModel>

class SortProxy : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    SortProxy(QObject *parent = nullptr);
    ~SortProxy() override;

protected:
    bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override;
};
#endif   // SORTPROXY_H
