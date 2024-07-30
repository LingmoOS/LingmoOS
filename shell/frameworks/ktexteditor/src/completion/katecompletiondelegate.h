/*
    SPDX-FileCopyrightText: 2022 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATECOMPLETIONDELEGATE_H
#define KATECOMPLETIONDELEGATE_H

// #include "expandingtree/expandingdelegate.h"
#include <QStyledItemDelegate>
#include <QTextLayout>

class KateCompletionWidget;
class KateCompletionModel;

class KateCompletionDelegate : public QStyledItemDelegate
{
public:
    explicit KateCompletionDelegate(QObject *parent);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QSize basicSizeHint(const QModelIndex &idx) const
    {
        return QStyledItemDelegate::sizeHint({}, idx);
    }

protected:
    static QList<QTextLayout::FormatRange> createHighlighting(const QModelIndex &index);

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // This variable is used by ArgumentHintDelegate to put the text at the top of the item so that
    // it isn't hidden by the expanding widget
    mutable bool m_alignTop = false;
};

#endif
