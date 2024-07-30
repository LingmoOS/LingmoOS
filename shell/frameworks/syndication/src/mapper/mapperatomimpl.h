/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_MAPPER_MAPPERATOMIMPL_H
#define SYNDICATION_MAPPER_MAPPERATOMIMPL_H

#include "feedatomimpl.h"

#include <atom/document.h>
#include <feed.h>
#include <mapper.h>
#include <specificdocument.h>

namespace Syndication
{
/** @internal */
class AtomMapper : public Mapper<Feed>
{
    QSharedPointer<Feed> map(SpecificDocumentPtr doc) const override
    {
        return QSharedPointer<Feed>(new FeedAtomImpl(doc.staticCast<Atom::FeedDocument>()));
    }
};

} // namespace Syndication

#endif // SYNDICATION_MAPPER_MAPPERATOMIMPL_H
