// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ItemDelegate_H
#define ItemDelegate_H

#include "notifymodel.h"

#include <QStyledItemDelegate>

class NotifyListView;

class ItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ItemDelegate(NotifyListView *view = Q_NULLPTR, NotifyModel * model = Q_NULLPTR, QObject *parent = Q_NULLPTR);

public:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool eventFilter(QObject *object, QEvent *event);

    QWidget *lastItemView() const;

Q_SIGNALS:
    void lastItemCreated();

private:
    NotifyModel *m_model;
    NotifyListView *m_view;
    mutable QPointer<QWidget> m_lastItemView;
};

#endif // ItemDelegate_H
