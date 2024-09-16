// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASE_DETAIL_ITEM_DELEGATE_H
#define BASE_DETAIL_ITEM_DELEGATE_H

#include <QStyledItemDelegate>

class BaseDetailItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit BaseDetailItemDelegate(QObject *parent = nullptr);

    virtual ~BaseDetailItemDelegate();

protected:
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // BASE_DETAIL_ITEM_DELEGATE_H
