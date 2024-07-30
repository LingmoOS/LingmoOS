/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_ATOM_ENTRY_H
#define SYNDICATION_ATOM_ENTRY_H

#include <syndication/elementwrapper.h>
#include <syndication/specificitem.h>

#include <QList>

#include <ctime>

class QDomElement;
class QString;

namespace Syndication
{
class SpecificItemVisitor;

namespace Atom
{
class Category;
class Content;
class Link;
class Person;
class Source;

/**
 * an Atom entry, equivalent to the "items" in the RSS world.
 *
 * @author Frank Osterfeld
 */
class SYNDICATION_EXPORT Entry : public ElementWrapper, public SpecificItem
{
public:
    /**
     * creates a null entry object
     */
    Entry();

    /**
     * creates an Entry object wrapping an atom:entry element.
     * @param element a DOM element, should be a atom:entry element
     * (although not enforced), otherwise this object will not parse
     * anything useful
     */
    explicit Entry(const QDomElement &element);

    /**
     * Used by visitors for double dispatch. See SpecificVisitor
     * for more information.
     * @param visitor the visitor calling the method
     */

    bool accept(SpecificItemVisitor *visitor) override;

    /**
     * list of persons who are authors of this entry. (required)
     *
     * If the entry itself does not have explicit author description,
     * its source author description is used. If none of these exist,
     * the list of authors of the containing feed is used. That list
     * is set by setFeedAuthors().
     */
    Q_REQUIRED_RESULT QList<Person> authors() const;

    /**
     * a list of categories this entry is filed to (optional)
     */
    Q_REQUIRED_RESULT QList<Category> categories() const;

    /**
     * list of persons contributing to this entry (optional)
     */
    Q_REQUIRED_RESULT QList<Person> contributors() const;

    /**
     * ID of the article. (required)
     * The ID must be unique inside this feed. The atom spec defines it as a
     * URI (which is not enforced by this parser)
     */
    Q_REQUIRED_RESULT QString id() const;

    /**
     * links pointing to associated web sites and other resources.
     *
     * Links are optional if the entry provides Content.
     * Otherwise, it must contain at least one link with
     * a @c rel value of @c "alternate". (see Link).
     */
    Q_REQUIRED_RESULT QList<Link> links() const;

    /**
     * copyright information (optional)
     *
     * @return copyright information for the entry (intended for human
     * readers), or a null string if not specified
     */
    Q_REQUIRED_RESULT QString rights() const;

    /**
     * source description of the content (optional)
     *
     * If the content was copied from another feed, this object contains
     * information about the source feed.
     *
     * @return source description, or a null object if not
     * specified
     */
    Q_REQUIRED_RESULT Source source() const;

    /**
     * The datetime of the publication of this entry (optional).
     *
     * @return the publication date in seconds since epoch
     */
    Q_REQUIRED_RESULT time_t published() const;

    /**
     * The datetime of the last modification of this entry (required).
     *
     * @return the modification date in seconds since epoch
     */
    Q_REQUIRED_RESULT time_t updated() const;

    /**
     * a short summary, abstract or excerpt of an entry. (optional)
     * This is usually more verbose than title() and but does not
     * contain the whole content as content() does.
     *
     * @return the summary as HTML, or a null string if not specified
     */
    Q_REQUIRED_RESULT QString summary() const;

    /**
     * title of the entry (required).
     *
     * @return the title as HTML
     */
    Q_REQUIRED_RESULT QString title() const;

    /**
     * content of the entry (optional)
     * See @ref Content for details
     *
     * @return entry content, or a null content object if not specified
     */
    Q_REQUIRED_RESULT Content content() const;

    /**
     * returns all child elements of this entry not covered by this class.
     * This can be used to access additional metadata from Atom extensions.
     */
    Q_REQUIRED_RESULT QList<QDomElement> unhandledElements() const;

    /**
     * Sets the list of the containing feed's authors, which will be used
     * as a fallback in authors() in case both the entry itself and its
     * source have no explicit author description.
     * @param feedAuthors the list of feed's authors
     */
    void setFeedAuthors(const QList<Person> &feedAuthors);

    /**
     * returns a description of this entry for debugging purposes
     *
     * @return debug string
     */
    Q_REQUIRED_RESULT QString debugInfo() const;

private:
    QList<Person> m_feedAuthors;
};

} // namespace Atom
} // namespace Syndication

#endif // SYNDICATION_ATOM_ENTRY_H
