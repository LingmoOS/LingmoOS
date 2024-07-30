/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_CATEGORY_H
#define SYNDICATION_CATEGORY_H

#include <QSharedPointer>
#include <QString>

#include "syndication_export.h"

namespace Syndication
{
class Category;
typedef QSharedPointer<Category> CategoryPtr;

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
class SYNDICATION_EXPORT Category
{
public:
    /**
     * destructor
     */
    virtual ~Category();

    /**
     * returns whether this object is a null category
     */
    virtual bool isNull() const = 0;

    /**
     * A term identifying the category, e.g. "general", "life", "books"
     * or "Basketball & other sport I like".
     * The term must be unique in its scheme (see scheme()).
     *
     * In user interfaces, use it only if there is no label() available.
     * TODO: specify format (HTML, plain text?) and enforce it in the impl
     * @return  category term. This string is never empty.
     */
    virtual QString term() const = 0;

    /**
     * An optional scheme the term is part of. This can be some
     * vocabulary/ontology such as Dublin Core.
     * Think of it as the term's namespace, grouping a set of categories.
     * When managing categories, scheme() + term() identifies a category
     * unambiguously and can be used as key.
     *
     * @return the scheme this category is part of, or a null string
     * if not specified
     */
    virtual QString scheme() const = 0;

    /**
     * An optional human-readable label of the category. If specified, this
     * string should be used to represent this category in a user interface.
     * If not specified, use term() instead.
     * TODO: specify format (HTML, plain text?) and enforce it in the impl
     * @return the label of this category, or a null string if not specified
     */
    virtual QString label() const = 0;

    /**
     * Description of the category for debugging purposes.
     *
     * @return debug string
     */
    virtual QString debugInfo() const;
};

} // namespace Syndication

#endif // SYNDICATION_CATEGORY_H
