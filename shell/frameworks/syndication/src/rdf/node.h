/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RDF_NODE_H
#define SYNDICATION_RDF_NODE_H

#include "../syndication_export.h"

#include <QSharedPointer>
#include <QString>

namespace Syndication
{
namespace RDF
{
class Model;
class Node;
class NodeVisitor;
//@cond PRIVATE
typedef QSharedPointer<Node> NodePtr;
//@endcond

/**
 * an RDF node, abstract baseclass for all RDF node types, like resources and
 * literals
 */
class Node
{
public:
    /**
     * destructor
     */
    virtual ~Node();

    /**
     * Used by visitors for double dispatch. See NodeVisitor
     * for more information.
     * @param visitor the visitor calling the method
     * @param ptr a shared pointer object for this node
     */
    virtual void accept(NodeVisitor *visitor, NodePtr ptr);

    /**
     * returns a copy of the object. Must be implemented
     * by subclasses to return a copy using the concrete
     * type
     */
    virtual Node *clone() const = 0;

    /**
     * returns whether this node is a null node
     */
    virtual bool isNull() const = 0;

    /**
     * returns whether this node is a resource
     */
    virtual bool isResource() const = 0;

    /**
     * returns whether this node is a property
     */
    virtual bool isProperty() const = 0;

    /**
     * returns whether this node is a literal
     */
    virtual bool isLiteral() const = 0;

    /**
     * returns whether this node is an RDF sequence
     */
    virtual bool isSequence() const = 0;

    /**
     * returns whether this node is an anonymous resource
     */
    virtual bool isAnon() const = 0;

    /**
     * the identifier of this node. the ID is unique per model
     * and set by the associated model at creation time.
     */
    virtual unsigned int id() const = 0;

    /**
     * returns a textual representation of the node.
     * This is the literal string for literals, and a null string for other
     * node types.
     */
    virtual QString text() const = 0;

    /**
     * used in Model
     * @internal
     */
    virtual void setModel(const Model &model) = 0;

    /**
     *  used in Model
     * @internal
     */
    virtual void setId(unsigned int id) = 0;

protected:
    /**
     * used to generate unique IDs for node objects
     */
    static unsigned int idCounter;
};

} // namespace RDF
} // namespace Syndication

#endif // SYNDICATION_RDF_NODE_H
