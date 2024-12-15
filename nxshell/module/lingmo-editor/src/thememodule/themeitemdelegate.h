// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THEMEITEMDELEGATE_H
#define THEMEITEMDELEGATE_H

#include <QAbstractItemDelegate>
#include <QPainterPath>

class ThemeItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    ThemeItemDelegate(QObject *parent = nullptr);
    ~ThemeItemDelegate();

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    int m_itemPaddingY = 8;
    int m_itemPaddingX = 20;
    int m_frameRadius = 5;
};

#endif
