// -*- c-basic-offset:4; indent-tabs-mode:nil -*-
/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2003 Alexander Kellett <lypanov@kde.org>
    SPDX-FileCopyrightText: 2008 Norbert Frese <nf2@scheinwelt.at>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kbookmarksettings_p.h"

#include <KConfig>
#include <KConfigGroup>

KBookmarkSettings *KBookmarkSettings::s_self = nullptr;

void KBookmarkSettings::readSettings()
{
    KConfig config(QStringLiteral("kbookmarkrc"), KConfig::NoGlobals);
    KConfigGroup cg(&config, QStringLiteral("Bookmarks"));

    // add bookmark dialog usage - no reparse
    s_self->m_advancedaddbookmark = cg.readEntry("AdvancedAddBookmarkDialog", false);

    // this one alters the menu, therefore it needs a reparse
    s_self->m_contextmenu = cg.readEntry("ContextMenuActions", true);
}

KBookmarkSettings *KBookmarkSettings::self()
{
    if (!s_self) {
        s_self = new KBookmarkSettings;
        readSettings();
    }
    return s_self;
}
