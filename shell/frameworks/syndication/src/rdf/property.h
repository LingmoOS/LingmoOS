/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RDF_PROPERTY_H
#define SYNDICATION_RDF_PROPERTY_H

#include <syndication/rdf/resource.h>

class QString;

namespace Syndication
{
namespace RDF
{
//@cond PRIVATE
class Property;
typedef QSharedPointer<Property> PropertyPtr;
//@endcond

/**
 * a property is node type that represents properties of things,
 * like "name" is a property of a person, or "color" is a property of e.g.
 * a car. Properties can be used as predicates in statements.
 *
 * @author Frank Osterfeld
 */
class Property : public Resource
{
public:
    /**
     * creates a null property
     */
    Property();

    /**
     * creates a property with a given URI
     *
     * @param uri the URI of the property
     */
    explicit Property(const QString &uri);

    /**
     * destructor
     */
    ~Property() override;

    /**
     * Used by visitors for double dispatch. See NodeVisitor
     * for more information.
     * @param visitor the visitor calling the method
     * @param ptr a shared pointer object for this node
     */
    void accept(NodeVisitor *visitor, NodePtr ptr) override;

    /**
     * returns true for properties
     */
    bool isProperty() const override;

    /**
     * creates a copy of the property object
     */
    Property *clone() const override;
};

} // namespace RDF
} // namespace Syndication

#endif // SYNDICATION_RDF_PROPERTY_H
