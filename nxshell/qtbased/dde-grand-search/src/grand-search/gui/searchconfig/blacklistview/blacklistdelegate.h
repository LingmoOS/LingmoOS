// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLACKLISTDELEGATE_H
#define BLACKLISTDELEGATE_H

#include <DListView>
#include <DStyledItemDelegate>

namespace GrandSearch {
class BlackListDelegate : public Dtk::Widget::DStyledItemDelegate
{
    Q_OBJECT
public:
    explicit BlackListDelegate(QAbstractItemView *parent = Q_NULLPTR);
    ~BlackListDelegate() override;

protected:
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void drawPathsText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawItemBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};
}

#endif // BLACKLISTDELEGATE_H
