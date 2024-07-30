/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1998, 1999 Torben Weis <weis@kde.org>
    SPDX-FileCopyrightText: 2006 Daniel Teske <teske@squorn.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KBOOKMARKACTIONINTERFACE_H
#define KBOOKMARKACTIONINTERFACE_H

#include "kbookmark.h"

/**
 * @class KBookmarkActionInterface kbookmarkactioninterface.h KBookmarkActionInterface
 *
 * The interface to implement by actions and menus which represent a bookimark
 */
class KBOOKMARKS_EXPORT KBookmarkActionInterface
{
public:
    explicit KBookmarkActionInterface(const KBookmark &bk);
    virtual ~KBookmarkActionInterface();
    const KBookmark bookmark() const;

private:
    const KBookmark bm;
};

#endif
