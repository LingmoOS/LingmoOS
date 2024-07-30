/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RSS2_TOOLS_P_H
#define SYNDICATION_RSS2_TOOLS_P_H

class QString;
template<class T>
class QList;

namespace Syndication
{
class ElementWrapper;

namespace RSS2
{
/**
 * @internal
 * extracts encoded content from XHTML, content:encoded and friends
 *
 * @param parent the parent node, e.g. a channel or item element
 * @return the content as HTML (see Item::content() for details)
 */
QString extractContent(const ElementWrapper &parent);

} // RSS2
} // Syndication

#endif // SYNDICATION_RSS2_TOOLS_P_H
