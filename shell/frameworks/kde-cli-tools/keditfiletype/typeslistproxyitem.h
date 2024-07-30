/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2022 Marco Rebhan <me@dblsaiko.net>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
*/

#ifndef TYPESLISTPROXYITEM_H
#define TYPESLISTPROXYITEM_H

#include "typeslistitembase.h"

class TypesListItem;
class QTreeWidget;

class TypesListProxyItem : public TypesListItemBase
{
public:
    explicit TypesListProxyItem(TypesListItem *inner, QTreeWidget *parent = nullptr);
    explicit TypesListProxyItem(TypesListItem *inner, QTreeWidgetItem *parent = nullptr);
    virtual ~TypesListProxyItem() override = default;

    virtual void loadIcon(bool forceReload) override;

    TypesListItem *inner();

private:
    TypesListItem *m_inner;
};

#endif
