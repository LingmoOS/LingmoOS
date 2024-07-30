/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_CONSTANTS_H
#define SYNDICATION_CONSTANTS_H

#include "syndication_export.h"

class QString;

namespace Syndication
{
SYNDICATION_EXPORT
QString xmlNamespace();

SYNDICATION_EXPORT
QString xhtmlNamespace();

SYNDICATION_EXPORT
QString dublinCoreNamespace();

SYNDICATION_EXPORT
QString contentNameSpace();

SYNDICATION_EXPORT
QString itunesNamespace();

/**
 * wellformedweb.org's RSS namespace for comment functionality
 * "http://wellformedweb.org/CommentAPI/"
 */
SYNDICATION_EXPORT
QString commentApiNamespace();

/**
 * "slash" namespace
 * http://purl.org/rss/1.0/modules/slash/
 */
SYNDICATION_EXPORT
QString slashNamespace();

} // namespace Syndication

#endif // SYNDICATION_CONSTANTS_H
