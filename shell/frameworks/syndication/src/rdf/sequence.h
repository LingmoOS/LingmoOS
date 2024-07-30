/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RDF_SEQUENCE_H
#define SYNDICATION_RDF_SEQUENCE_H

#include "../syndication_export.h"
#include "resource.h"

template<class T>
class QList;

namespace Syndication
{
namespace RDF
{
//@cond PRIVATE
class Sequence;
typedef QSharedPointer<Sequence> SequencePtr;
//@endcond

/**
 * Sequence container, a sequence contains an ordered list
 * of RDF nodes. (opposed to the usually unordered graph
 * structure)
 */
class Sequence : public Resource
{
public:
    /**
     * creates a null sequence
     */
    Sequence();

    /**
     * creates a sequence with the given URI. Do not use this directly,
     * use Model::createSequence() instead.
     */
    explicit Sequence(const QString &uri);

    /**
     * copies a sequence
     *
     * @param other sequence
     */
    Sequence(const Sequence &other);

    /**
     * destructor
     */
    ~Sequence() override;

    /**
     * assigns another sequence
     *
     * @param other the sequence to assign
     */
    virtual Sequence &operator=(const Sequence &other);

    /**
     * Used by visitors for double dispatch. See NodeVisitor
     * for more information.
     * @param visitor the visitor calling the method
     * @param ptr a shared pointer object for this node
     */
    void accept(NodeVisitor *visitor, NodePtr ptr) override;

    /**
     * creates a copy of the sequence
     */
    Sequence *clone() const override;

    /**
     * appends a node at the end of the sequence
     *
     * @param node the RDF node to append to the sequence
     */
    virtual void append(NodePtr node);

    /**
     * the list of the list items in the sequence, in the
     * specified order
     */
    virtual QList<NodePtr> items() const;

    /**
     * returns @p true
     */
    bool isSequence() const override;

private:
    class SequencePrivate;
    QSharedPointer<SequencePrivate> d;
};

} // namespace RDF
} // namespace Syndication

#endif // SYNDICATION_RDF_SEQUENCE_H
