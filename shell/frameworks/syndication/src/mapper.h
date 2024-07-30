/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_MAPPER_H
#define SYNDICATION_MAPPER_H

#include "syndication_export.h"

namespace Syndication
{
class SpecificDocument;
//@cond PRIVATE
typedef QSharedPointer<SpecificDocument> SpecificDocumentPtr;
//@endcond

/**
 * @brief A mapper maps an SpecificDocument to something else.
 * The type of this "something else" is specified by the template
 * parameter T.
 * In the default implementation it is used with the Feed interface,
 * but it is not limited to that. T can be an arbitrary class.
 *
 * There are three (advanced and hopefully rare) use cases
 * that require you to implement your own mapper.
 * For more information on the possible uses, see TODO: link docs.
 *
 * 1) Add your own feed parser. In case you need support for another
 * feed format (Okay! News, CDF, completely backward-incompatible Atom 5.0,
 * you name it), you can
 * implement AbstractParser and SpecificDocument for it and provide a
 * Mapper&lt;Feed>
 *
 * * @code
 * class OkayNewsMapper : public Mapper<Feed>
 * {
 *     public:
 *
 *     virtual FeedPtr map(SpecificDocumentPtr doc) const { ... }
 * };
 *
 * parserCollection()->registerParser(new OkayNews::Parser, new OkayNewsMapper);
 * @endcode
 *
 * 2) Implement your own mapper for the Feed abstraction, for an
 * existing parser. E.g. if you think Syndication does map Atom
 * all wrong, you can implement your own Atom mapper and use that instead
 * of the default one.
 *
 * @code
 * class MyAtomMapper : public Mapper<Feed>
 * {
 *     public:
 *
 *     virtual FeedPtr map(SpecificDocumentPtr doc) const { ... }
 * };
 *
 * parserCollection()->changeMapper("atom", new MyAtomMapper);
 * @endcode
 *
 * 3) Use your own abstraction. In case the Feed interface
 * does not fit your needs, you can use your own interface, let's
 * say "MyFeed". Be aware you have to implement custom mappings for
 * all feed formats then:
 *
 * @code
 * class MyFeed
 * {
 *     public:
 *
 *     QString title() const; // my special title
 *     QList<Article> articles() const; // I name it articles
 * };
 *
 * class MyAtomMapper : public Mapper<MyFeed> { ... };
 * class MyRDFMapper : public Mapper<MyFeed> { ... };
 * class MyRSS2Mapper : public Mapper<MyFeed> { ... };
 *
 * ParserCollection<MyFeed>* coll = new ParserCollection<MyFeed>;
 * coll->registerParser(new Atom::Parser, new MyAtomMapper);
 * coll->registerParser(new RDF::Parser, new MyRDFMapper);
   coll->registerParser(new RSS2::Parser, new MyRSS2Mapper);
 * @endcode
 *
 * @author Frank Osterfeld
 */
template<class T>
class SYNDICATION_EXPORT Mapper
{
public:
    /**
     * virtual destructor
     */
    virtual ~Mapper()
    {
    }

    /**
     * maps a format-specific document to abstraction of type
     * @c T.
     *
     * \note implementations may assume @c doc to have the
     * type whose mapping they implement and may just statically cast
     * to the subclass without further checking. If you register your
     * own mapper, it's your responsibility to register the mapper
     * only for the format it actually handles.
     *
     * @param doc the document to map.
     * @return a newly created object implementing the abstraction
     * @c T.
     */
    virtual QSharedPointer<T> map(SpecificDocumentPtr doc) const = 0;
};

} // namespace syndication

#endif // SYNDICATION_MAPPER_H
