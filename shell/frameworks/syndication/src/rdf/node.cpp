/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "node.h"
#include "nodevisitor.h"

namespace Syndication
{
namespace RDF
{
unsigned int Node::idCounter = 1;

void Node::accept(NodeVisitor *visitor, NodePtr ptr)
{
    visitor->visitNode(ptr);
}

Node::~Node()
{
}

} // namespace RDF
} // namespace Syndication
