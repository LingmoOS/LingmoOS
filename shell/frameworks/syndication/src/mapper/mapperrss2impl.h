/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_MAPPERRSS2IMPL_H
#define SYNDICATION_MAPPERRSS2IMPL_H

#include "feedrss2impl.h"

#include <feed.h>
#include <mapper.h>
#include <rss2/document.h>
#include <specificdocument.h>

namespace Syndication
{
/** @internal */
class RSS2Mapper : public Mapper<Feed>
{
    QSharedPointer<Feed> map(SpecificDocumentPtr doc) const override
    {
        return QSharedPointer<Feed>(new FeedRSS2Impl(doc.staticCast<RSS2::Document>()));
    }
};

} // namespace Syndication

#endif // SYNDICATION_MAPPERRSS2IMPL_H
