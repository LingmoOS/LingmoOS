//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Alexander Kellett <lypanov@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __kbookmarksettings_p_h__
#define __kbookmarksettings_p_h__

#include <kbookmarks_export.h>

class QString;
class KBookmark;
class KBookmarkGroup;

#define KEDITBOOKMARKS_BINARY "keditbookmarks"

class KBOOKMARKS_EXPORT KBookmarkSettings
{
public:
    bool m_advancedaddbookmark;
    bool m_contextmenu;
    static KBookmarkSettings *s_self;
    static void readSettings();
    static KBookmarkSettings *self();
};

#endif
