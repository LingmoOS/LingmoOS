/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2022 Marco Rebhan <me@dblsaiko.net>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
*/

#ifndef TYPESLISTITEMBASE_H
#define TYPESLISTITEMBASE_H

#include <QTreeWidgetItem>

class TypesListItemBase : public QTreeWidgetItem
{
public:
    TypesListItemBase(QTreeWidget *parent = nullptr);
    TypesListItemBase(QTreeWidgetItem *parent = nullptr);

    virtual ~TypesListItemBase() override = default;

    virtual void loadIcon(bool forceReload = false) = 0;
};

#endif
