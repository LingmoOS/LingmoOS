/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2000-2008 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2008 Urs Wolfer <uwolfer @ kde.org>
    SPDX-FileCopyrightText: 2022 Marco Rebhan <me@dblsaiko.net>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "typeslisttreewidget.h"

#include "typeslistitembase.h"
#include <QKeyEvent>

TypesListTreeWidget::TypesListTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{
}

void TypesListTreeWidget::drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    static_cast<TypesListItemBase *>(itemFromIndex(index))->loadIcon();

    QTreeWidget::drawRow(painter, option, index);
}

void TypesListTreeWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space && selectionMode() == ExtendedSelection) {
        // for some reason pressing space only (de)selects one of the items
        // in the tree widget, this reimplements the behavior so that it
        // toggles every selected item

        QTreeWidgetItemIterator it(this);
        bool first = true;
        Qt::CheckState state = Qt::Unchecked;

        while (*it) {
            QTreeWidgetItem *item = *it;

            if (item->isSelected() && item->data(0, Qt::CheckStateRole).isValid()) {
                if (first) {
                    state = item->checkState(0);

                    if (item->checkState(0) != Qt::Checked) {
                        state = Qt::Checked;
                    } else {
                        state = Qt::Unchecked;
                    }

                    first = false;
                }

                item->setCheckState(0, state);
            }

            ++it;
        }

        event->accept();
    } else {
        QTreeWidget::keyPressEvent(event);
    }
}

#include "moc_typeslisttreewidget.cpp"
