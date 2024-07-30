/*
    SPDX-FileCopyrightText: 2012 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NOSELECTIONRECTDELEGATEEGATE_H
#define NOSELECTIONRECTDELEGATEEGATE_H

#include <QStyledItemDelegate>

#include <kcupslib_export.h>

class KCUPSLIB_EXPORT NoSelectionRectDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit NoSelectionRectDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // NOSELECTIONRECTDELEGATEEGATE_H
