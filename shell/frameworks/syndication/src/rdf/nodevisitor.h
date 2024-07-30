/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RDF_NODEVISITOR_H
#define SYNDICATION_RDF_NODEVISITOR_H

#include <QSharedPointer>

#include "../syndication_export.h"

namespace Syndication
{
namespace RDF
{
//@cond PRIVATE
class Literal;
typedef QSharedPointer<Literal> LiteralPtr;
class Node;
typedef QSharedPointer<Node> NodePtr;
class Property;
typedef QSharedPointer<Property> PropertyPtr;
class Resource;
typedef QSharedPointer<Resource> ResourcePtr;
class Sequence;
typedef QSharedPointer<Sequence> SequencePtr;
//@endcond

/**
 * Visitor interface, following the Visitor design pattern. Use this if you
 * want to process nodes and the way how to handle the nodes depends
 * on it's concrete type (e.g. Resource or Literal).
 *
 * TODO: insert code example
 *
 * @author Frank Osterfeld
 */
class NodeVisitor // krazy:exclude=dpointer
{
public:
    /**
     * destructor
     */
    virtual ~NodeVisitor();

    /**
     * call this method to handle a node. Depending on the concrete type
     * of the node, a specialized visit method is called.
     *
     * @param node the node to process
     */
    virtual void visit(NodePtr node);

    /**
     * reimplement this method to handle literals.
     *
     * @param item the literal to visit
     * @return whether the visitor handled the literal.
     * Reimplementations of this method must return @p true.
     */
    virtual bool visitLiteral(LiteralPtr);

    /**
     * reimplement this method to handle nodes that weren't handled
     * by the more specific method.
     *
     * @param node the node to visit
     * @return whether the visitor handled the node.
     * Reimplementations of this method must return @p true.
     */
    virtual bool visitNode(NodePtr node);

    /**
     * reimplement this method to handle properties.
     *
     * @param property the property to visit
     * @return whether the visitor handled the property.
     * Reimplementations of this method must return @p true.
     */
    virtual bool visitProperty(PropertyPtr property);

    /**
     * reimplement this method to handle resources.
     *
     * @param resource the resource to visit
     * @return whether the visitor handled the resource.
     * Reimplementations of this method must return @p true.
     */
    virtual bool visitResource(ResourcePtr resource);

    /**
     * reimplement this method to handle sequences.
     *
     * @param seq the sequence to visit
     * @return whether the visitor handled the sequence.
     * Reimplementations of this method must return @p true.
     */
    virtual bool visitSequence(SequencePtr seq);
};

} // namespace RDF
} // namespace Syndication

#endif // SYNDICATION_RDF_NODEVISITOR_H
