/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1998, 1999 Torben Weis <weis@kde.org>
    SPDX-FileCopyrightText: 2006 Daniel Teske <teske@squorn.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kbookmarkactioninterface.h"

KBookmarkActionInterface::KBookmarkActionInterface(const KBookmark &bk)
    : bm(bk)
{
}

KBookmarkActionInterface::~KBookmarkActionInterface()
{
}

const KBookmark KBookmarkActionInterface::bookmark() const
{
    return bm;
}
