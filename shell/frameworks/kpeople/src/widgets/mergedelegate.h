/*
    KPeople - Duplicates
    SPDX-FileCopyrightText: 2013 Franck Arrecot <franck.arrecot@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef MERGEDELEGATE_H
#define MERGEDELEGATE_H

#include <kpeople/widgets/kpeoplewidgets_export.h>

#include <KExtendableItemDelegate>
#include <QItemSelection>

class QAbstractItemView;
class QItemSelection;

class KPEOPLEWIDGETS_EXPORT MergeDelegate : public KExtendableItemDelegate
{
    Q_OBJECT

public:
    explicit MergeDelegate(QAbstractItemView *parent);
    ~MergeDelegate() override;

    static QSize pictureSize();

public Q_SLOTS:
    void onClickContactParent(const QModelIndex &parent);
    void onSelectedContactsChanged(const QItemSelection &now, const QItemSelection &old);

private:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    KPEOPLEWIDGETS_NO_EXPORT QWidget *buildMultipleLineLabel(const QModelIndex &idx);

    static QSize s_arrowSize;
    static QSize s_decorationSize;
};

#endif // MERGEDELEGATE_H
