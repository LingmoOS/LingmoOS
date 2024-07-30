/*
    SPDX-FileCopyrightText: 2002-2008 The Kopete developers <kopete-devel@kde.org>
    SPDX-FileCopyrightText: 2008 Carlo Segato <brandon.ml@gmail.com>
    SPDX-FileCopyrightText: 2002-2003 Stefan Gehn <metz@gehn.net>
    SPDX-FileCopyrightText: 2005 Engin AYDOGAN <engin@bzzzt.biz>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KEMOTICONSPARSER_P_H
#define KEMOTICONSPARSER_P_H

class QString;

/** ASCII art smily replacement with Unicode emojis.
 *  Taken from former KEmoticons, which has been deprecated for KF6.
 */
namespace KEmoticonsParser
{
/**
 * Parses emoticons in text @p text.
 * @param text the text to parse
 * @return the text with emoticons replaced by Unicode emojis
 */
QString parseEmoticons(const QString &text);
}

#endif
