/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "specificitemvisitor.h"
#include "specificitem.h"

namespace Syndication
{
SpecificItemVisitor::~SpecificItemVisitor()
{
}

bool SpecificItemVisitor::visit(SpecificItem *item)
{
    return item->accept(this);
}

bool SpecificItemVisitor::visitRSS2Item(Syndication::RSS2::Item *)
{
    return false;
}

bool SpecificItemVisitor::visitRDFItem(Syndication::RDF::Item *)
{
    return false;
}

bool SpecificItemVisitor::visitAtomEntry(Syndication::Atom::Entry *)
{
    return false;
}

} // namespace Syndication
