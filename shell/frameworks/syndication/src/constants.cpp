/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "constants.h"

#include <QString>

namespace Syndication
{
QString xmlNamespace()
{
    return QStringLiteral("http://www.w3.org/XML/1998/namespace");
}

QString xhtmlNamespace()
{
    return QStringLiteral("http://www.w3.org/1999/xhtml");
}

QString contentNameSpace()
{
    return QStringLiteral("http://purl.org/rss/1.0/modules/content/");
}

QString dublinCoreNamespace()
{
    return QStringLiteral("http://purl.org/dc/elements/1.1/");
}

QString itunesNamespace()
{
    return QStringLiteral("http://www.itunes.com/dtds/podcast-1.0.dtd");
}

QString commentApiNamespace()
{
    return QStringLiteral("http://wellformedweb.org/CommentAPI/");
}

QString slashNamespace()
{
    return QStringLiteral("http://purl.org/rss/1.0/modules/slash/");
}

} // namespace Syndication
