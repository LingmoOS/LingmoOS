// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ITEMDELEGATE_H
#define ITEMDELEGATE_H

#include <QModelIndex>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

class ItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

Q_SIGNALS:

public:
    explicit ItemDelegate(QObject *parent = nullptr);
    ~ItemDelegate();

    // 重写绘画函数
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    static QStringList m_unit;
};

#endif   // ITEMDELEGATE_H
