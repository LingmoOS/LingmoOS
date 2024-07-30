/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RSS2_CATEGORY_H
#define SYNDICATION_RSS2_CATEGORY_H

#include <syndication/elementwrapper.h>

class QDomElement;
class QString;

namespace Syndication
{
namespace RSS2
{
/**
 * A category which can be assigned to items or whole feeds.
 * These can be simple tags as known from delicious or Technorati, or
 * a category from a hierarchical taxonomy or ontology.
 *
 * @author Frank Osterfeld
 */
class Category : public ElementWrapper
{
public:
    /**
     * Creates a Category object wrapping a @c &lt;category> XML element.
     *
     * @param element The @c &lt;category> element to wrap
     */
    explicit Category(const QDomElement &element);

    /**
     * Default constructor, creates a null object, for which isNull() is
     * @c true.
     */
    Category();

    /**
     * Name of the category. This is both to be used as identifier and as
     * human-readable string. It can bea forward-slash-separated string
     * to identify a hierarchic location in the domain indicated by
     * domain(). Examples: "General", "Programming", "Funny",
     * "Books/History".
     *
     * @return The category identifier/name as string or a null string for
     * null objects.
     *
     */
    QString category() const;

    /**
     * optional, identifies the domain of the category, i.e. a
     * categorization taxonomy.
     *
     * @return The domain of the category, or a null string if none is set
     * (and for null objects)
     */
    QString domain() const;

    /**
     * Returns a description of the object for debugging purposes.
     *
     * @return debug string
     */
    QString debugInfo() const;
};

} // namespace RSS2
} // namespace Syndication

#endif // SYNDICATION_RSS2_CATEGORY_H
