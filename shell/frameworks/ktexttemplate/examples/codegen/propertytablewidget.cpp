/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "propertytablewidget.h"

#include <QHeaderView>
#include <QMenu>

#include "comboboxdelegate.h"
#include <qevent.h>

PropertyTableWidget::PropertyTableWidget(QWidget *parent)
    : QTableWidget(parent)
{
    QStringList types;
    for (int i = 0; i < sizeof sTypes / sizeof *sTypes; ++i)
        types << *(sTypes + i);

    setItemDelegateForColumn(0, new ComboBoxDelegate(types, ComboBoxDelegate::Editable));

    setEditTriggers(QAbstractItemView::AllEditTriggers);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void PropertyTableWidget::contextMenuEvent(QContextMenuEvent *contextMenuEvent)
{
    QMenu *popup = new QMenu(this);
    QAction *action;
    action = popup->addAction("Add");
    connect(action, SIGNAL(triggered(bool)), SLOT(slotAdd()));
    action = popup->addAction("Clear All");
    connect(action, SIGNAL(triggered(bool)), SLOT(slotClear()));
    QModelIndex index = indexAt(contextMenuEvent->pos());
    if (index.isValid()) {
        action = popup->addAction("Remove");
        connect(action, SIGNAL(triggered(bool)), SLOT(slotRemove()));
    }
    popup->exec(contextMenuEvent->globalPos());
}

void PropertyTableWidget::slotAdd()
{
    int row = rowCount();
    insertRow(row);
    setItem(row, 0, new QTableWidgetItem(QString()));
    setItem(row, 1, new QTableWidgetItem(QString()));

    QTableWidgetItem *checkableItem = new QTableWidgetItem;
    checkableItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    checkableItem->setCheckState(Qt::Unchecked);
    setItem(row, 2, checkableItem);
}

void PropertyTableWidget::slotRemove()
{
    removeRow(currentIndex().row());
}

void PropertyTableWidget::slotClear()
{
    for (int row = rowCount() - 1; row >= 0; --row)
        removeRow(row);
}

#include "moc_propertytablewidget.cpp"
