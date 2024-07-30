/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_SPECIFICITEM_H
#define SYNDICATION_SPECIFICITEM_H

#include <QSharedPointer>

#include "syndication_export.h"

namespace Syndication
{
class SpecificItemVisitor;
class SpecificItem;

//@cond PRIVATE
typedef QSharedPointer<SpecificItem> SpecificItemPtr;
//@endcond

/**
 * Interface for all format-specific item-like classes, such as
 * RSS2/RDF items, and Atom entries. To process items based on their
 * format, use a SpecificItemVisitor.
 *
 * @author Frank Osterfeld
 */
class SYNDICATION_EXPORT SpecificItem
{
public:
    /**
     * virtual dtor
     */
    virtual ~SpecificItem();

    /**
     * This must be implemented for the double dispatch
     * technique (Visitor pattern).
     *
     * The usual implementation is
     * @code
     * return visitor->visit(this);
     * @endcode
     *
     * See also SpecificItemVisitor.
     *
     * @param visitor the visitor "visiting" this object
     */
    virtual bool accept(SpecificItemVisitor *visitor) = 0;
};

} // namespace Syndication

#endif // SYNDICATION_SPECIFICITEM_H
