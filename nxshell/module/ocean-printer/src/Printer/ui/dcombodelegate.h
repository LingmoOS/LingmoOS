// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCOMBODELEGATE_H
#define DCOMBODELEGATE_H

#include <QItemDelegate>
#include <QPainter>
#include <QStyledItemDelegate>

enum COMBOITEMROLE {
    VALUEROLE = Qt::UserRole + 1,
    BACKCOLORROLE
};

enum ITEMSTATE {
    NORMAL,
    CONFLICT
};

/*
class DComboDelegate:public QItemDelegate
{
public:
    DComboDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index) const override;
};
*/

/*
class ItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT

signals:
    void deleteItem(const QModelIndex &index);

public:
    ItemDelegate(QObject * parent= nullptr);
    ~ItemDelegate(){}
    void paint(QPainter * painter,const QStyleOptionViewItem & option,const QModelIndex & index) const;
};
*/

class ComItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    explicit ComItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
};

#endif // DCOMBODELEGATE_H
