/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RSS2_ITEM_H
#define SYNDICATION_RSS2_ITEM_H

#include <syndication/elementwrapper.h>
#include <syndication/rss2/document.h>
#include <syndication/specificitem.h>

#include <ctime>

class QDomElement;
class QString;
template<class T>
class QList;

namespace Syndication
{
class SpecificItemVisitor;

namespace RSS2
{
class Category;
class Enclosure;
class Source;

/**
 * An Item, representing an entry in an RSS feed.
 *
 * @author Frank Osterfeld
 */
class Item : public ElementWrapper, public Syndication::SpecificItem
{
public:
    /**
     * Default constructor, creates a null object, for which isNull() is
     * @c true.
     */
    explicit Item(QSharedPointer<Document> doc = QSharedPointer<Document>());

    /**
     * Creates an Item object wrapping an @c &lt;item> XML element.
     *
     * @param element The @c &lt;item> element to wrap
     * @param doc the document this item is part of
     */
    explicit Item(const QDomElement &element, QSharedPointer<Document> doc = QSharedPointer<Document>());

    /**
     * creates a copy of an item. As the d pointer is shared,
     * this is a cheap operation.
     *
     * @param other the item to copy
     */
    Item(const Item &other);

    /**
     * destructor
     */
    ~Item() override;

    /**
     * assigns another item. As the d pointer is shared,
     * this is a cheap operation.
     *
     * @param other the item to assign
     */
    Item &operator=(const Item &other);

    /**
     * Used by visitors for double dispatch. See SpecificItemVisitor
     * for more information.
     * @param visitor the visitor calling the method
     */
    bool accept(SpecificItemVisitor *visitor) override;

    /**
     * The title of the item.
     *
     * @return The title in plain text. Note that e.g. characters like <,
     * >, & are not escaped!
     * (TODO: this might change, check what makes more sense)
     */
    QString title() const;

    /**
     * The URL of the item. This usually links to the web representation
     * of the item, e.g. the full news article.
     *
     * @return an URL, or a null string if not set
     */
    QString link() const;

    /**
     * The item synopsis. This might contain a short summary of the
     * item, but also the full content. If content() is set, that usually
     * contains the full content instead.
     *
     * @return a string in HTML format (whitespace is irrelevant,
     * @c &lt;br/> is used for newlines, "&", "&lt;", "&gt;" are escaped)
     * summarizing the item. A null string if no description was specified.
     */
    QString description() const;

    /**
     * Returns the actual content of the item. In RSS2, this can be stored
     * in various elements, e.g. in content:encoded, xhtml:body or
     * xhtml:div. If this is not set, description() might also contain the
     * content of the item.
     *
     * @return the content in HTML format (whitespace is irrelevant,
     * &lt;br/> is used for newlines, "&", "&lt;", "&gt;" are escaped)
     * If no content is specified, a null string is returned.
     */
    QString content() const;

    /**
     * Set of categories this item is included in.
     *
     * @return a list of categories, possibly empty.
     */
    QList<Category> categories() const;

    /**
     * URL of a page for comments relating to the item.
     *
     * @return an URL to the comments, or a null string if not set
     */
    QString comments() const;

    /**
     * The email address of the author of this item. For newspapers and
     * magazines syndicating via RSS, the author is the person who wrote
     * the article that this item describes. For collaborative weblogs, the
     * author of the item might be different from the managing editor or
     * webmaster.
     * This method returns the content of the @c &lt;author> element. If
     * @c &lt;author> is not available, the method returns
     * @c &lt;dc:creator> instead, if available.
     *
     * @return an email address of the author, or a null string if not
     * specified
     */
    QString author() const;

    /**
     * Descriptions of media objects that are attached to the item.
     * Note that the RSS2 spec is a bit unclear about whether an item can
     * have multiple enclosures or not. Originally it was not intended, but
     * in reality, some tools out there specify multiple enclosures.
     * So most of the time, this list be either empty or contains a
     * single item, but don't take that for granted
     */
    QList<Enclosure> enclosures() const;

    /**
     * "guid stands for globally unique identifier. It's a string that
     * uniquely identifies the item. When present, an aggregator may choose
     * to use this string to determine if an item is new.
     * There are no rules for the syntax of a guid. Aggregators must view
     * them as a string. It's up to the source of the feed to establish the
     * uniqueness of the string."
     *
     * @return a guid string, or a null string if none specified in the
     * feed
     */
    QString guid() const;

    /**
     * If @c true, it can be assumed that the guid is a permalink to the
     * item, that is, a url that can be opened in a Web browser, that
     * points to the full item.
     *
     * @return @c true if the guid is a permalink and can be interpreted as
     * URL
     */
    bool guidIsPermaLink() const;

    /**
     * Indicates when the item was published. If it's a date in the future,
     * you may choose to not display the item until that date.
     * This returns the content of the @c &lt;pubDate> element. If @c
     * &lt;pubDate> is not available, the method returns
     * @c &lt;dc:date> instead, if available.
     *
     * @return the publication date, or 0 if no date was specified or
     * parsing failed
     */
    time_t pubDate() const;

    /**
     * expiration date, specifying a date when the item is not longer
     * available.
     * Only available in RSS 0.93.
     *
     * @return the expiration date, or 0 if no date was specified or
     * parsing failed
     */
    time_t expirationDate() const;

    /**
     * A Platform for Internet Content Selection (PICS) rating tag.
     * More information on the format of the rating tag can be found here:
     * http://www.w3.org/PICS/
     *
     * @return PICS rating information, or a null string if not specified
     */
    QString rating() const;

    /**
     * The RSS channel that the item came from. See Source class for more
     * information.
     *
     * @return a Source object, or a null object (see Source.isNull()) if
     * not set.
     */
    Source source() const;

    /**
     * returns all child elements of this item not covered by this class.
     * You can use this to access additional metadata from RSS extensions.
     */
    QList<QDomElement> unhandledElements() const;

    /**
     * Returns a description of the object and its children for debugging
     * purposes.
     *
     * @return debug string
     */
    QString debugInfo() const;

    //@cond PRIVATE
    /**
     * @internal
     * returns the description content in unmodified form (no
     * plaintext/HTML conversions etc.)
     */
    QString originalDescription() const;

    /**
     * @internal
     * returns the title content in unmodified form (no
     * plaintext/HTML conversions etc.)
     */
    QString originalTitle() const;
    //@endcond

private:
    class ItemPrivate;
    QSharedPointer<ItemPrivate> d;
};

} // namespace RSS2
} // namespace Syndication

#endif // SYNDICATION_RSS2_ITEM_H
