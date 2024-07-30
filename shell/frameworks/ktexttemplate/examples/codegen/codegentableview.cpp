/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "codegentableview.h"

#include "comboboxdelegate.h"
#include <QHeaderView>

MethodTableView::MethodTableView(QWidget *parent)
    : QTableView(parent)
{
    QStringList accessTypes;
    accessTypes << "public"
                << "protected"
                << "private"
                << "signals"
                << "public slots"
                << "protected slots"
                << "private slots"
                << "Q_PRIVATE_SLOT";

    setItemDelegateForColumn(0, new ComboBoxDelegate(accessTypes));

    QStringList types;
    types << "void";
    for (int i = 0; i < sizeof sTypes / sizeof *sTypes; ++i)
        types << *(sTypes + i);

    setItemDelegateForColumn(2, new ComboBoxDelegate(types, ComboBoxDelegate::Editable));

    setEditTriggers(QAbstractItemView::AllEditTriggers);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

ArgsTableView::ArgsTableView(QWidget *parent)
    : QTableView(parent)
{
    QStringList types;
    for (int i = 0; i < sizeof sTypes / sizeof *sTypes; ++i)
        types << *(sTypes + i);

    setItemDelegateForColumn(0, new ComboBoxDelegate(types, ComboBoxDelegate::Editable));

    setEditTriggers(QAbstractItemView::AllEditTriggers);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

#include "moc_codegentableview.cpp"
