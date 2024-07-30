/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2022 Marco Rebhan <me@dblsaiko.net>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
*/

#include "typeslistproxyitem.h"

#include "typeslistitem.h"

TypesListProxyItem::TypesListProxyItem(TypesListItem *inner, QTreeWidget *parent)
    : TypesListItemBase(parent)
    , m_inner(inner)
{
    setText(0, inner->text(0));
}

TypesListProxyItem::TypesListProxyItem(TypesListItem *inner, QTreeWidgetItem *parent)
    : TypesListItemBase(parent)
    , m_inner(inner)
{
    setText(0, inner->text(0));
}

void TypesListProxyItem::loadIcon(bool forceReload)
{
    m_inner->loadIcon(forceReload);

    if ((!m_inner->mimeTypeData().icon().isEmpty() && icon(0).isNull()) || forceReload) {
        QTreeWidgetItem::setIcon(0, QIcon::fromTheme(m_inner->mimeTypeData().icon()));
    }
}

TypesListItem *TypesListProxyItem::inner()
{
    return m_inner;
}
