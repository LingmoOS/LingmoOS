/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_ATOM_SOURCE_H
#define SYNDICATION_ATOM_SOURCE_H

#include <syndication/elementwrapper.h>

#include <ctime>

class QDomElement;
class QString;

template<class T>
class QList;

namespace Syndication
{
namespace Atom
{
class Category;
class Generator;
class Link;
class Person;

/**
 * If an entry was copied from another feed, this class contains
 * a description of the source feed.
 *
 * @author Frank Osterfeld
 */
class SYNDICATION_EXPORT Source : public ElementWrapper
{
public:
    /**
     * creates a null source object
     */
    Source();

    /**
     * creates a Source object wrapping a atom:source element.
     *
     * @param element a DOM element, should be a atom:source element
     * (although not enforced), otherwise this object will not parse
     * anything useful
     */
    explicit Source(const QDomElement &element);

    /**
     * authors of the original content (optional)
     */
    Q_REQUIRED_RESULT QList<Person> authors() const;

    /**
     * contributors to the original content (optional)
     */
    Q_REQUIRED_RESULT QList<Person> contributors() const;

    /**
     * categories the source feed is assigned to (optional)
     */
    Q_REQUIRED_RESULT QList<Category> categories() const;

    /**
     * description of the software which generated the source feed
     * (optional)
     */
    Q_REQUIRED_RESULT Generator generator() const;

    /**
     * URL of an image serving as a feed icon (optional)
     *
     * @return icon URL, or a null string if not specified
     */
    Q_REQUIRED_RESULT QString icon() const;

    /**
     * a string that unambiguously identifies the source feed (optional)
     *
     * @return the ID of the source feed, or a null string if not
     * specified.
     */
    Q_REQUIRED_RESULT QString id() const;

    /**
     * a list of links. See Link for more information on
     * link types.
     */
    Q_REQUIRED_RESULT QList<Link> links() const;

    /**
     * URL of an image, the logo of the source feed (optional)
     *
     * @return image URL, or a null string if not specified in the feed.
     */
    Q_REQUIRED_RESULT QString logo() const;

    /**
     * copyright information (optional)
     *
     * @return copyright information for the source,
     * or a null string if not specified
     */
    Q_REQUIRED_RESULT QString rights() const;

    /**
     * description or subtitle of the source feed (optional).
     *
     * @return subtitle string as HTML, or a null string
     * if not specified.
     */
    Q_REQUIRED_RESULT QString subtitle() const;

    /**
     * source feed title (optional).
     *
     * @return title string as HTML, or a null string if not specified
     */
    Q_REQUIRED_RESULT QString title() const;

    /**
     * The datetime of the last modification of the source feed
     * content. (optional)
     *
     * @return the modification date in seconds since epoch
     */
    Q_REQUIRED_RESULT time_t updated() const;

    /**
     * description of this source object for debugging purposes
     *
     * @return debug string
     */
    Q_REQUIRED_RESULT QString debugInfo() const;
};

} // namespace Atom
} // namespace Syndication

#endif // SYNDICATION_ATOM_SOURCE_H
