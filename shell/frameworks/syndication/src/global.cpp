/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "global.h"
#include "documentsource.h"
#include "parsercollectionimpl.h"

#include "atom/parser.h"
#include "mapper/mapperatomimpl.h"
#include "mapper/mapperrdfimpl.h"
#include "mapper/mapperrss2impl.h"
#include "rdf/parser.h"
#include "rss2/parser.h"

#include <QCoreApplication>

namespace
{
static bool collectionIsInitialized = false;
}

namespace Syndication
{
static ParserCollectionImpl<Syndication::Feed> *parserColl = nullptr;

namespace
{
// only executed when then was a QCoreApplication
static void cleanupParserCollection()
{
    delete parserColl;
    parserColl = nullptr;
}

} // namespace

ParserCollection<Feed> *parserCollection()
{
    if (!collectionIsInitialized) {
        parserColl = new ParserCollectionImpl<Syndication::Feed>;
        qAddPostRoutine(cleanupParserCollection);
        parserColl->registerParser(new RSS2::Parser, new RSS2Mapper);
        parserColl->registerParser(new Atom::Parser, new AtomMapper);
        parserColl->registerParser(new RDF::Parser, new RDFMapper);
        collectionIsInitialized = true;
    }
    return parserColl;
}

FeedPtr parse(const DocumentSource &src, const QString &formatHint)
{
    return parserCollection()->parse(src, formatHint);
}

} // namespace Syndication
