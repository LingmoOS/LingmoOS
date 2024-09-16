// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SIDEBARTREEWIDGETITEMDELEGATE_H
#define SIDEBARTREEWIDGETITEMDELEGATE_H

#include <QStyledItemDelegate>

class SideBarTreeWidgetItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    SideBarTreeWidgetItemDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

#endif // SIDEBARTREEWIDGETITEMDELEGATE_H
