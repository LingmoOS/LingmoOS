/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "literal.h"
#include "nodevisitor.h"

namespace Syndication
{
namespace RDF
{
class SYNDICATION_NO_EXPORT Literal::LiteralPrivate
{
public:
    QString text;
    unsigned int id;

    bool operator==(const LiteralPrivate &other) const
    {
        return text == other.text;
    }
};

Literal::Literal()
    : d()
{
}

Literal::Literal(const Literal &other)
    : Node(other)
{
    d = other.d;
}

Literal *Literal::clone() const
{
    return new Literal(*this);
}

void Literal::accept(NodeVisitor *visitor, NodePtr ptr)
{
    LiteralPtr lptr = ptr.staticCast<Syndication::RDF::Literal>();
    if (!visitor->visitLiteral(lptr)) {
        Node::accept(visitor, ptr);
    }
}

Literal::Literal(const QString &text)
    : d(new LiteralPrivate)
{
    d->text = text;
    d->id = idCounter++;
}

Literal::~Literal()
{
}

Literal &Literal::operator=(const Literal &other)
{
    d = other.d;
    return *this;
}

bool Literal::operator==(const Literal &other) const
{
    if (!d || !other.d) {
        return d == other.d;
    }

    return *d == *(other.d);
}

bool Literal::isNull() const
{
    return !d;
}

unsigned int Literal::id() const
{
    return d ? d->id : 0;
}

bool Literal::isResource() const
{
    return false;
}

bool Literal::isProperty() const
{
    return false;
}

bool Literal::isLiteral() const
{
    return true;
}

bool Literal::isAnon() const
{
    return false;
}

bool Literal::isSequence() const
{
    return false;
}

QString Literal::text() const
{
    return d ? d->text : QString();
}

Literal::operator QString() const
{
    return d ? d->text : QString();
}

void Literal::setModel(const Model & /*model*/)
{
}

void Literal::setId(unsigned int id)
{
    d->id = id;
}

} // namespace RDF
} // namespace Syndication
