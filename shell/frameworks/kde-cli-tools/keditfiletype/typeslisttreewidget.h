/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2000-2008 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2008 Urs Wolfer <uwolfer @ kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TYPESLISTTREEWIDGET_H
#define TYPESLISTTREEWIDGET_H

#include <QTreeWidget>

// helper class for loading the icon on request instead of preloading lots of probably
// unused icons which takes quite a lot of time
class TypesListTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit TypesListTreeWidget(QWidget *parent = nullptr);

protected:
    void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void keyPressEvent(QKeyEvent *event) override;
};

#endif
