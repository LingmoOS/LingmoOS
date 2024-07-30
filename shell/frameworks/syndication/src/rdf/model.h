/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RDF_MODEL_H
#define SYNDICATION_RDF_MODEL_H

#include "../syndication_export.h"

#include "document.h"
#include "literal.h"
#include "node.h"
#include "property.h"
#include "resource.h"
#include "sequence.h"
#include "statement.h"

#include <QString>

template<class T>
class QList;

namespace Syndication
{
namespace RDF
{
/**
 * An RDF model, a set of RDF statements.
 * Model objects are implicitly shared.
 *
 * @author Frank Osterfeld
 */
class Model
{
    friend class ::Syndication::RDF::Document;
    friend class ::Syndication::RDF::Document::Private;
    friend class ::Syndication::RDF::Resource;
    friend class ::Syndication::RDF::Resource::ResourcePrivate;
    friend class ::Syndication::RDF::Statement;
    friend class ::Syndication::RDF::Statement::StatementPrivate;

public:
    /**
     * default constructor, creates an empty model
     * containing no statements
     */
    Model();

    /**
     * constructs a model from another.
     * Both models will share the same set of statements,
     * so adding/removing statements from one model also
     * modifies the other!
     *
     * @param other another model
     */
    Model(const Model &other);

    /**
     * destructor
     */
    virtual ~Model();

    /**
     * assigns another model. Both models will share the same
     * set of statements, so adding/removing statements from
     * one model also modifies the other!
     *
     * @param other another model
     */
    Model &operator=(const Model &other);

    /**
     * Returns whether two models objects represent the same model
     * (i.e. share the same underlying statement set). Currently this
     * method does _not_ compare the statement list.
     * Two independently created models containing the same statements
     * are not equal!
     *
     * @param other the model to compare to
     */
    bool operator==(const Model &other) const;

    /**
     * creates a resource and associates it with this model. If the model
     * already contains a resource with the given URI, the existing instance
     * is returned.
     *
     * @param uri the URI of the resource. If a null string, a blank node
     * is created.
     * @return a shared pointer to the requested resource
     */
    virtual ResourcePtr createResource(const QString &uri = QString());

    /**
     * creates a property and associates it with this model. If the model
     * already contains a property with the given URI, the existing instance
     * is returned.
     *
     * @param uri the URI of the property. This must be non-empty, otherwise
     * null property is returned
     * @return a shared pointer to the requested property
     */
    virtual PropertyPtr createProperty(const QString &uri);

    /**
     * creates a sequence and associates it with this model. If the model
     * already contains a sequence with the given URI, the existing
     * instance is returned.
     *
     * @param uri the URI of the sequence, or a null string for an
     * anonymous instance
     * @return a shared pointer to the requested sequence
     */
    virtual SequencePtr createSequence(const QString &uri = QString());

    /**
     * creates a literal and associates it with this model.
     *
     * @param text the literal text
     * @return a shared pointer to the requested literal
     */
    virtual LiteralPtr createLiteral(const QString &text);

    /**
     * adds a statement to the model.
     *
     * @param subject
     * @param predicate
     * @param object
     * @return a shared pointer to a statement associated with this
     * model, with the given @c subject, @c predicate and @c object
     */
    virtual StatementPtr addStatement(ResourcePtr subject, PropertyPtr predicate, NodePtr object);

    /**
     * removes a statement from the model.
     *
     * @param subject subject of the statement
     * @param predicate predicate of the statement
     * @param object object of the statement
     */
    virtual void removeStatement(ResourcePtr subject, PropertyPtr predicate, NodePtr object);

    /**
     * removes a statement from the model.
     *
     * @param statement the statement to remove
     */
    virtual void removeStatement(StatementPtr statement);

    /**
     * returns whether this model is empty, i.e. contains no statements.
     */
    virtual bool isEmpty() const;

    /**
     * returns all resources of a given type.
     * subClassOf semantics are ignored.
     *
     * @param type a resource representing an RDFS class
     */
    virtual QList<ResourcePtr> resourcesWithType(ResourcePtr type) const;

    /**
     * returns a list of the statements in this model.
     *
     */
    virtual QList<StatementPtr> statements() const;

    /**
     * searches the model for a node by ID.
     *
     * @param id the ID to search for
     * @return the node with the given ID, or a null node (which is of type
     * Literal) if the model doesn't contain the node with this ID
     */
    virtual NodePtr nodeByID(uint id) const;

    /**
     * searches the model for a resource by ID.
     *
     * @param id the ID to search for
     * @return the resource with the given ID, or a null resource if the
     * model doesn't contain a resource with this ID
     */
    virtual ResourcePtr resourceByID(uint id) const;

    /**
     * searches the model for a property by ID.
     *
     * @param id the ID to search for
     * @return the property with the given ID, or a null property if the
     * model doesn't contain a property with this ID
     */
    virtual PropertyPtr propertyByID(uint id) const;

    /**
     * searches the model for a literal by ID.
     *
     * @param id the ID to search for
     * @return the literal with the given ID, or a null literal if the
     * model doesn't contain a literal with this ID
     */
    virtual LiteralPtr literalByID(uint id) const;
    //@cond PRIVATE
    /**
     * @internal
     * used by Resource::hasProperty()
     */
    virtual bool resourceHasProperty(const Resource *resource, PropertyPtr property) const;

    /**
     * @internal
     * used by Resource::property()
     */
    virtual StatementPtr resourceProperty(const Resource *resource, PropertyPtr property) const;

    /**
     * @internal
     * used by Resource::properties()
     */
    virtual QList<StatementPtr> resourceProperties(const Resource *resource, PropertyPtr property) const;

    //@endcond
    /**
     * a debug string listing the contained statements for
     * debugging purposes
     *
     * @return debug string
     */
    virtual QString debugInfo() const;

private:
    class ModelPrivate;
    QSharedPointer<ModelPrivate> d;
};

} // namespace RDF
} // namespace Syndication

#endif // SYNDICATION_RDF_MODEL_H
