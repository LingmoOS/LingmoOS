/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_FEED_H
#define SYNDICATION_FEED_H

#include <QSharedPointer>

#include "syndication_export.h"

class QDomElement;

template<class T>
class QList;
template<class K, class T>
class QMultiMap;
class QString;

namespace Syndication
{
//@cond PRIVATE
class SpecificDocument;
typedef QSharedPointer<SpecificDocument> SpecificDocumentPtr;
class Category;
typedef QSharedPointer<Category> CategoryPtr;
class Feed;
typedef QSharedPointer<Feed> FeedPtr;
class Image;
typedef QSharedPointer<Image> ImagePtr;
class Item;
typedef QSharedPointer<Item> ItemPtr;
class Person;
typedef QSharedPointer<Person> PersonPtr;
//@endcond

/**
 * This class represents a feed document ("Channel" in RSS, "Feed" in Atom).
 * It contains a ordered list of items (e.g., articles) and a description of the
 * feed (title, homepage, etc.). This interface abstracts from format-specific
 * details of e.g. Atom::FeedDocument or RSS::Document and provides a
 * format-agnostic, unified view on the document.
 * This way applications using the syndication library have no need to care
 * about the syndication format jungle at all. If necessary, format details and
 * specialities can be accessed using the specificDocument() method.
 *
 * @author Frank Osterfeld
 */
class SYNDICATION_EXPORT Feed
{
public:
    /**
     * destructor
     */
    virtual ~Feed();

    /**
     * returns the format-specific document this abstraction wraps.
     * If you want to access format-specific properties, this can be used,
     * in combination with a DocumentVisitor.
     *
     * @return a shared pointer to the wrapped document.
     */
    virtual SpecificDocumentPtr specificDocument() const = 0;

    /**
     * A list of items, in the order they were parsed from the feed source.
     * (usually reverse chronological order, see also Item::datePublished()
     * for sorting purposes).
     *
     * @return list of items
     */
    virtual QList<ItemPtr> items() const = 0;

    /**
     * returns a list of categories this feed is associated with.
     * See Category for more information.
     *
     */
    virtual QList<CategoryPtr> categories() const = 0;

    /**
     * The title of the feed.
     *
     * This string may contain HTML markup.(Importantly, occurrences of
     * the characters &lt;,'\n', '&amp;', '\'' and  '\"' are escaped).
     *
     * @return the title, or a null string if none is specified
     */
    virtual QString title() const = 0;

    /**
     * returns a link pointing to a website associated with this channel.
     * (blog, news site etc.)
     *
     * @return a WWW link, or a null string if none is specified
     */
    virtual QString link() const = 0;

    /**
     * A description of the feed.
     *
     * This string may contain HTML markup.(Importantly, occurrences of
     * the characters &lt;,'\n', '&amp;', '\'' and  '\"' are escaped).
     *
     * @return the description as HTML, or a null string if none is
     * specified
     */
    virtual QString description() const = 0;

    /**
     * returns an image associated with this item.
     *
     * @return an image object, or a null image (Not a null pointer!
     * I.e., image()->isNull() is @c true)
     * if no image is specified in the feed
     *
     */
    virtual ImagePtr image() const = 0;

    /**
     * returns an icon associated with this item.
     *
     * @return an icon object, or a null icon (Not a null pointer!
     * I.e., icon()->isNull() is @c true)
     * if no image is specified in the feed
     *
     */
    virtual ImagePtr icon() const = 0;

    /**
     * returns a list of persons who created the feed content. If there is a
     * distinction between authors and contributors (Atom), both are added
     * to the list, where authors are added first.
     *
     * @return list of authors (and possibly other contributing persons)
     */
    virtual QList<PersonPtr> authors() const = 0;

    /**
     * The language used in the feed. This is a global setting, which can
     * be overridden by the contained items.
     *
     * TODO: describe concrete format (language codes)
     */
    virtual QString language() const = 0;

    /**
     * returns copyright information about the feed
     */
    virtual QString copyright() const = 0;

    /**
     * returns a list of feed metadata not covered by this class.
     * Can be used e.g. to access format extensions.
     *
     * The returned map contains key value pairs, where the key
     * is the tag name of the element, namespace prefix are resolved
     * to the corresponding URIs. The value is the XML element as read
     * from the document.
     *
     * For example, to access the &lt;itunes:subtitle> element, use
     * additionalProperties()["http://www.itunes.com/dtds/podcast-1.0.dtdsubtitle"].
     *
     * Currently this is only
     * supported for RSS 0.91..0.94/2.0 and Atom formats, but not for RDF
     * (RSS 0.9 and 1.0).
     */
    virtual QMultiMap<QString, QDomElement> additionalProperties() const = 0;

    /**
     * returns a description of the feed for debugging purposes
     *
     * @return debug string
     */
    virtual QString debugInfo() const;
};

} // namespace Syndication

#endif // SYNDICATION_FEED_H
