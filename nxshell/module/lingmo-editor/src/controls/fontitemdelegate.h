// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FONTITEMDELEGATE_H
#define FONTITEMDELEGATE_H

#include <QStyledItemDelegate>

class FontItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit FontItemDelegate(QObject *parent = nullptr);
    ~FontItemDelegate();

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif
