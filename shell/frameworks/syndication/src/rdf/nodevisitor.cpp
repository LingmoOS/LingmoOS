/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "nodevisitor.h"

#include "literal.h"
#include "node.h"
#include "property.h"
#include "resource.h"
#include "sequence.h"

namespace Syndication
{
namespace RDF
{
NodeVisitor::~NodeVisitor()
{
}

void NodeVisitor::visit(NodePtr node)
{
    node->accept(this, node);
}

bool NodeVisitor::visitLiteral(LiteralPtr)
{
    return false;
}

bool NodeVisitor::visitNode(NodePtr)
{
    return false;
}

bool NodeVisitor::visitProperty(PropertyPtr)
{
    return false;
}

bool NodeVisitor::visitResource(ResourcePtr)
{
    return false;
}

bool NodeVisitor::visitSequence(SequencePtr)
{
    return false;
}
} // namespace RDF
} // namespace Syndication
