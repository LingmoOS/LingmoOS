/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2022 Marco Rebhan <me@dblsaiko.net>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
*/

#include "typeslistitembase.h"

TypesListItemBase::TypesListItemBase(QTreeWidget *parent)
    : QTreeWidgetItem(parent)
{
}

TypesListItemBase::TypesListItemBase(QTreeWidgetItem *parent)
    : QTreeWidgetItem(parent)
{
}
