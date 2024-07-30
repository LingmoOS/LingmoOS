/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "parser.h"
#include "document.h"

#include <documentsource.h>

#include <QDomDocument>
#include <QString>

namespace Syndication
{
namespace RSS2
{
class SYNDICATION_NO_EXPORT Parser::ParserPrivate
{
};

bool Parser::accept(const Syndication::DocumentSource &source) const
{
    QDomDocument doc = source.asDomDocument();
    if (doc.isNull()) {
        return false;
    }

    QDomNode root = doc.namedItem(QStringLiteral("rss")).toElement();

    return !root.isNull();
}

Syndication::SpecificDocumentPtr Parser::parse(const DocumentSource &source) const
{
    return DocumentPtr(new Document(Document::fromXML(source.asDomDocument())));
}

QString Parser::format() const
{
    return QStringLiteral("rss2");
}

Parser::Parser()
    : d(nullptr)
{
    Q_UNUSED(d) // silence -Wunused-private-field
}

Parser::Parser(const Parser &other)
    : AbstractParser(other)
    , d(nullptr)
{
}
Parser::~Parser() = default;

Parser &Parser::operator=(const Parser & /*other*/)
{
    return *this;
}

} // namespace RSS2
} // namespace Syndication
