/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_ATOM_CATEGORY_H
#define SYNDICATION_ATOM_CATEGORY_H

#include <syndication/elementwrapper.h>

class QDomElement;
class QString;

namespace Syndication
{
namespace Atom
{
/**
 * A category for categorizing items or whole feeds.
 * A category can be an informal string set by the feed author ("General",
 * "Stuff I like"), a tag assigned by readers, as known from flickr.com
 * or de.licio.us ("KDE", "funny"), or a term from a formally defined ontology.
 *
 * To represent the category in a user interface, use label() (or term() as
 * fallback). To create a key for e.g. storage purposes, use scheme() + term().
 *
 * @author Frank Osterfeld
 */
class SYNDICATION_EXPORT Category : public ElementWrapper
{
public:
    /**
     * creates a null category object.
     */
    Category();

    /**
     * creates a Category object wrapping an atom:category element.
     * @param element a DOM element, should be a atom:category element
     * (although not enforced), otherwise this object will not parse
     * anything useful
     */
    explicit Category(const QDomElement &element);

    /**
     * a term describing the category. (required)
     *
     * @return the category term as plain text (no HTML, "&", "<" etc. are
     * unescaped!)
     */
    Q_REQUIRED_RESULT QString term() const;

    /**
     * naming scheme the category term is part of. (optional)
     * A term is unique in its scheme (like in C++ identifiers are
     * unique in their namespaces)
     *
     * @return a URI representing the scheme, or a null string
     * if not specified
     */
    Q_REQUIRED_RESULT QString scheme() const;

    /**
     * Label of the category (optional).
     * If specified, this string should be used to represent this category
     * in a user interface.
     * If not specified, use term() instead.
     *
     * @return the label as plain text (no HTML, "&", "<" etc. are
     * unescaped!), or a null string if not specified
     */
    Q_REQUIRED_RESULT QString label() const;

    /**
     * description of this category object for debugging purposes
     *
     * @return debug string
     */
    Q_REQUIRED_RESULT QString debugInfo() const;
};

} // namespace Atom
} // namespace Syndication

#endif // SYNDICATION_ATOM_CATEGORY_H
