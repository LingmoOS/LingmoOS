/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sequence.h"
#include "node.h"
#include "nodevisitor.h"

#include <QList>
#include <QString>

namespace Syndication
{
namespace RDF
{
class SYNDICATION_NO_EXPORT Sequence::SequencePrivate
{
public:
    QList<NodePtr> items;
};

Sequence::Sequence()
    : Resource()
    , d()
{
}

Sequence::Sequence(const QString &uri)
    : Resource(uri)
    , d(new SequencePrivate)
{
}

Sequence::Sequence(const Sequence &other)
    : Resource(other)
{
    *this = other;
}

Sequence::~Sequence()
{
}
void Sequence::accept(NodeVisitor *visitor, NodePtr ptr)
{
    SequencePtr sptr = ptr.staticCast<Sequence>();
    if (!visitor->visitSequence(sptr)) {
        Resource::accept(visitor, ptr);
    }
}

Sequence *Sequence::clone() const
{
    return new Sequence(*this);
}

Sequence &Sequence::operator=(const Sequence &other)
{
    Resource::operator=(other);
    d = other.d;
    return *this;
}

void Sequence::append(NodePtr node)
{
    if (d) {
        d->items.append(node);
    }
}

QList<NodePtr> Sequence::items() const
{
    return d ? d->items : QList<NodePtr>();
}

bool Sequence::isSequence() const
{
    return true;
}

} // namespace RDF
} // namespace Syndication
