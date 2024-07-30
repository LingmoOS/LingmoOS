/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RDF_RESOURCE_H
#define SYNDICATION_RDF_RESOURCE_H

#include <syndication/rdf/node.h>

class QString;

template<class T>
class QList;

namespace Syndication
{
namespace RDF
{
class Model;
class Property;
typedef QSharedPointer<Property> PropertyPtr;
class Resource;
class Statement;
typedef QSharedPointer<Statement> StatementPtr;

typedef QSharedPointer<Resource> ResourcePtr;

/**
 * Resources are the entities in the RDF graph.
 * In RSS, e.g. the feed channel itself and the items are
 * resources.
 */
class Resource : public Node
{
    friend class Model;

public:
    /**
     * creates a null resource
     */
    Resource();

    /**
     * copies a resource
     *
     * @param other the resource to copy
     */
    Resource(const Resource &other);

    /**
     * creates a resource with a given URI.
     * Do not use this directly, use Model::createResource() instead.
     *
     * @param uri the URI of the new resource
     */
    explicit Resource(const QString &uri);

    /**
     * destructor
     */
    ~Resource() override;

    /**
     * assigns a resource
     *
     * @param other the resource to assign
     */
    Resource &operator=(const Resource &other);

    /**
     * checks two resources for equality. Currently both URI (or anonID)
     * _and_ id() must be equal!
     *
     * @param other the node to compare this node to
     */
    bool operator==(const Resource &other) const;

    /**
     * Used by visitors for double dispatch. See NodeVisitor
     * for more information.
     * @param visitor the visitor calling the method
     * @param ptr a shared pointer object for this node
     */
    void accept(NodeVisitor *visitor, NodePtr ptr) override;

    /**
     * creates a copy of the resource object
     */
    Resource *clone() const override;

    /**
     * the model this resource belongs to
     */
    virtual Model model() const;

    /**
     * returns whether the resource has a property @p property in the
     * associated model.
     *
     * @param property the property to check for
     */
    virtual bool hasProperty(PropertyPtr property) const;

    /**
     * returns a statement from the associated model where this resource is
     * the subject and the given property the predicate.
     *
     * @param property the property to check for
     *
     * @return the first statement found that satisfies the conditions.
     * If there are multiple statements, an arbitrary one is returned.
     */
    virtual StatementPtr property(PropertyPtr property) const;

    /**
     * returns the list of all statements from the associated model where
     * this resource is the subject and the given property the predicate.
     *
     * @param property the property to check for
     *
     * @return a list of the statements that satisfy the conditions.
     */
    virtual QList<StatementPtr> properties(PropertyPtr property) const;

    /**
     * returns whether the resource is a null resource
     */
    bool isNull() const override;

    /**
     * the identifier of this node. the ID is unique per model
     * and set by the associated model at creation time.
     */
    unsigned int id() const override;

    /**
     * returns @p true
     */
    bool isResource() const override;

    /**
     * returns @p false
     */
    bool isLiteral() const override;

    /**
     * returns @p true if this resource is also a property, @p false
     * otherwise
     */
    bool isProperty() const override;

    /**
     * returns whether this resource is an anonymous resource
     */
    bool isAnon() const override;

    /**
     * returns @p true if this resource is also a sequence, @p false
     * otherwise.
     */
    bool isSequence() const override;

    /**
     * returns a null string
     */
    QString text() const override;

    /**
     * returns the URI of the resource
     */
    virtual QString uri() const;

    /**
     * used in Model
     * @internal
     */
    void setModel(const Model &model) override;

    /**
     * used in Model
     * @internal
     */
    void setId(unsigned int id) override;

private:
    class ResourcePrivate;
    typedef QSharedPointer<ResourcePrivate> ResourcePrivatePtr;
    ResourcePrivatePtr d;
};

} // namespace RDF
} // namespace Syndication

#endif // SYNDICATION_RDF_RESOURCE_H
