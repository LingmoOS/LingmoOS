/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "documentvisitor.h"
#include "specificdocument.h"

namespace Syndication
{
DocumentVisitor::~DocumentVisitor()
{
}

bool DocumentVisitor::visit(SpecificDocument *document)
{
    return document->accept(this);
}

bool DocumentVisitor::visitRSS2Document(Syndication::RSS2::Document *)
{
    return false;
}

bool DocumentVisitor::visitRDFDocument(Syndication::RDF::Document *)
{
    return false;
}

bool DocumentVisitor::visitAtomFeedDocument(Syndication::Atom::FeedDocument *)
{
    return false;
}

bool DocumentVisitor::visitAtomEntryDocument(Syndication::Atom::EntryDocument *)
{
    return false;
}

} // namespace Syndication
