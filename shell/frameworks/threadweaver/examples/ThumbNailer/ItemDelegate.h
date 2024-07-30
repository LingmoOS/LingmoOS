/* -*- C++ -*-
    This file declares the SMIVItemDelegate class.

    SPDX-FileCopyrightText: 2005 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    License: LGPL with the following explicit clarification:
        This code may be linked against any version of the Qt toolkit
        from Trolltech, Norway.

    $Id: SMIVItemDelegate.h 30 2005-08-16 16:16:04Z mirko $
*/

#ifndef ITEMDELEGATE_H
#define ITEMDELEGATE_H

#include <QItemDelegate>
#include <QSize>

class ItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    ItemDelegate(QObject *parent = nullptr);
    static const int FrameWidth;
    static const int TextMargin;
    static const int Margin;

private:
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // SMIVITEMDELEGATE
