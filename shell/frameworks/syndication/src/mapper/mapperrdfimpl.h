/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_MAPPER_MAPPERRDFIMPL_H
#define SYNDICATION_MAPPER_MAPPERRDFIMPL_H

#include "feedrdfimpl.h"

#include <feed.h>
#include <mapper.h>
#include <rdf/document.h>
#include <specificdocument.h>

namespace Syndication
{
/** @internal */
class RDFMapper : public Mapper<Feed>
{
    QSharedPointer<Feed> map(SpecificDocumentPtr doc) const override
    {
        return QSharedPointer<Feed>(new FeedRDFImpl(doc.staticCast<RDF::Document>()));
    }
};

} // namespace Syndication

#endif // SYNDICATION_MAPPER_MAPPERRDFIMPL_H
