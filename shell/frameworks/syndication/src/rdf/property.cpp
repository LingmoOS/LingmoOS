/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "property.h"
#include "nodevisitor.h"

namespace Syndication
{
namespace RDF
{
Property::Property()
    : Resource()
{
}

Property::Property(const QString &uri)
    : Resource(uri)
{
}

Property::~Property()
{
}

bool Property::isProperty() const
{
    return true;
}

void Property::accept(NodeVisitor *visitor, NodePtr ptr)
{
    PropertyPtr pptr = ptr.staticCast<Syndication::RDF::Property>();

    if (!visitor->visitProperty(pptr)) {
        Resource::accept(visitor, ptr);
    }
}

Property *Property::clone() const
{
    return new Property(*this);
}

} // namespace RDF
} // namespace Syndication
