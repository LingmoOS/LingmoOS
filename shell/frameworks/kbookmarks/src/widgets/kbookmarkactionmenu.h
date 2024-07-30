/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1998, 1999 Torben Weis <weis@kde.org>
    SPDX-FileCopyrightText: 2006 Daniel Teske <teske@squorn.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KBOOKMARKACTIONMENU_H
#define KBOOKMARKACTIONMENU_H

#include "kbookmarkactioninterface.h"
#include <KActionMenu>
#include <kbookmarkswidgets_export.h>

/**
 * @class KBookmarkActionMenu kbookmarkactionmenu.h KBookmarkActionMenu
 *
 * A wrapper around KActionMenu to provide a nice constructor for bookmark groups.
 */
class KBOOKMARKSWIDGETS_EXPORT KBookmarkActionMenu : public KActionMenu, public KBookmarkActionInterface
{
    Q_OBJECT
public:
    KBookmarkActionMenu(const KBookmark &bm, QObject *parent);
    KBookmarkActionMenu(const KBookmark &bm, const QString &text, QObject *parent);
    ~KBookmarkActionMenu() override;
};

#endif
