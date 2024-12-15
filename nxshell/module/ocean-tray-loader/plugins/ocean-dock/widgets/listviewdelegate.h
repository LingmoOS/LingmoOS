// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LISTVIEWDELEGATE_H
#define LISTVIEWDELEGATE_H

#include <DStyledItemDelegate>

DWIDGET_USE_NAMESPACE

class ListViewDelegate : public DStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ListViewDelegate(QAbstractItemView *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // LISTVIEWDELEGATE_H
