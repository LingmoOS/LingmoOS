/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1998, 1999 Torben Weis <weis@kde.org>
    SPDX-FileCopyrightText: 2006 Daniel Teske <teske@squorn.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kbookmarkactionmenu.h"

KBookmarkActionMenu::KBookmarkActionMenu(const KBookmark &bm, QObject *parent)
    : KActionMenu(QIcon::fromTheme(bm.icon()), bm.text().replace(QLatin1Char('&'), QLatin1String("&&")), parent)
    , KBookmarkActionInterface(bm)
{
    setToolTip(bm.description());
    setIconText(text());
}

KBookmarkActionMenu::KBookmarkActionMenu(const KBookmark &bm, const QString &text, QObject *parent)
    : KActionMenu(text, parent)
    , KBookmarkActionInterface(bm)
{
}

KBookmarkActionMenu::~KBookmarkActionMenu()
{
}

#include "moc_kbookmarkactionmenu.cpp"
