/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_ATOM_DOCUMENT_H
#define SYNDICATION_ATOM_DOCUMENT_H

#include <syndication/elementwrapper.h>
#include <syndication/specificdocument.h>

#include <ctime>

template<class T>
class QList;

namespace Syndication
{
class DocumentVisitor;

namespace Atom
{
class Category;
class Entry;
class EntryDocument;
class FeedDocument;
class Generator;
class Link;
class Person;
//@cond PRIVATE
typedef QSharedPointer<EntryDocument> EntryDocumentPtr;
typedef QSharedPointer<FeedDocument> FeedDocumentPtr;
//@endcond

/**
 * An Atom 1.0 Feed Document, containing metadata describing the
 * feed and a number of entries.
 *
 * @author Frank Osterfeld
 */
class SYNDICATION_EXPORT FeedDocument : public Syndication::SpecificDocument, public ElementWrapper
{
public:
    /**
     * default constructor, creates a null feed, which
     * is invalid.
     * @see isValid()
     */
    FeedDocument();

    /**
     * creates a FeedDocument wrapping an atom:feed element.
     * @param element a DOM element, should be a atom:feed document
     * (although not enforced), otherwise this object will not parse
     * anything useful
     */
    explicit FeedDocument(const QDomElement &element);

    /**
     * Used by visitors for double dispatch. See DocumentVisitor
     * for more information.
     * @param visitor the visitor calling the method
     */
    bool accept(DocumentVisitor *visitor) override;

    /**
     * a list of persons who are the authors of this feed.
     * According to the Atom 1.0 spec, a feed must have an
     * author unless all entries in it have one.
     */
    Q_REQUIRED_RESULT QList<Person> authors() const;

    /**
     * a list of persons who contribute to this feed. (optional)
     */
    Q_REQUIRED_RESULT QList<Person> contributors() const;

    /**
     * a list of categories this feed is assigned to (optional)
     */
    Q_REQUIRED_RESULT QList<Category> categories() const;

    /**
     * URL of an image serving as a feed icon (optional)
     *
     * @return icon URL, or a null string if not specified in the feed.
     */
    Q_REQUIRED_RESULT QString icon() const;

    /**
     * URL of an image serving as a feed logo (optional)
     *
     * @return image URL, or a null string if not specified in the feed.
     */
    Q_REQUIRED_RESULT QString logo() const;

    /**
     * a string that unambiguously identifies the feed (required)
     *
     * @return the ID of the feed. As defined in the Atom spec it must be
     * a valid URI (which is neither checked nor enforced by this parser)
     *
     */
    Q_REQUIRED_RESULT QString id() const;

    /**
     * copyright information (optional)
     *
     * @return copyright information for the feed (intended for human
     * readers), or a null string if not specified
     */
    Q_REQUIRED_RESULT QString rights() const;

    /**
     * feed title (required).
     *
     * @return title string as HTML.
     */
    Q_REQUIRED_RESULT QString title() const;

    /**
     * description or subtitle of the feed (optional).
     *
     * @return subtitle string as HTML, or a null string
     * if not specified in the feed.
     */
    Q_REQUIRED_RESULT QString subtitle() const;

    /**
     * description of the agent used to generate the feed. See
     * Generator for more information (optional).
     *
     * @return description of the generator, or a null Generator object
     * if not specified in the feed.
     */
    Q_REQUIRED_RESULT Generator generator() const;

    /**
     * The datetime of the last modification of the feed content.
     *
     * @return the modification date in seconds since epoch
     */
    Q_REQUIRED_RESULT time_t updated() const;

    /**
     * a list of links. See Link for more information on
     * link types.
     */
    Q_REQUIRED_RESULT QList<Link> links() const;

    /**
     * a list of the entries (items) in this feed.
     */
    Q_REQUIRED_RESULT QList<Entry> entries() const;

    /**
     * returns all child elements of this feed not covered by this class.
     * This can be used to access additional metadata from Atom extensions.
     */
    Q_REQUIRED_RESULT QList<QDomElement> unhandledElements() const;

    /**
     * returns a description of this feed document for debugging
     * purposes.
     *
     * @return debug string
     */
    Q_REQUIRED_RESULT QString debugInfo() const override;

    /**
     * returns whether this document is valid or not.
     * Invalid documents do not contain any useful
     * information.
     */
    Q_REQUIRED_RESULT bool isValid() const override;
};

/**
 * An Atom 1.0 Entry Document, containing a single Atom entry outside
 * of the context of a feed.
 *
 * @author Frank Osterfeld
 */
class SYNDICATION_EXPORT EntryDocument : public Syndication::SpecificDocument, public Syndication::ElementWrapper
{
public:
    /**
     * default constructor, creates a null document, which is invalid.
     * @see isValid()
     */
    EntryDocument();

    /**
     * creates an Atom Entry Document wrapping an atom:entry element.
     * @param element a DOM element, should be a atom:entry element
     * (although not enforced), otherwise this object will not parse
     * anything useful
     */
    explicit EntryDocument(const QDomElement &element);

    /**
     * Used by visitors for double dispatch. See DocumentVisitor
     * for more information.
     * @param visitor the visitor calling the method
     */
    bool accept(DocumentVisitor *visitor) override;

    /**
     * returns the single entry described in the source.
     *
     * @return the entry
     */
    Q_REQUIRED_RESULT Entry entry() const;

    /**
     * returns a description of this entry document for debugging
     * purposes.
     *
     * @return debug string
     */
    Q_REQUIRED_RESULT QString debugInfo() const override;

    /**
     * returns whether this document is valid or not.
     * Invalid documents do not contain any useful
     * information.
     */
    Q_REQUIRED_RESULT bool isValid() const override;
};

} // namespace Atom
} // namespace Syndication

#endif // SYNDICATION_ATOM_DOCUMENT_H
