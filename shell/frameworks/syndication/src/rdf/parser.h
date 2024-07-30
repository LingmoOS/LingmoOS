/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RDF_PARSER_H
#define SYNDICATION_RDF_PARSER_H

#include <syndication/abstractparser.h>
#include <syndication/rdf/document.h>

#include <memory>

namespace Syndication
{
class DocumentSource;

namespace RDF
{
/**
 * Parser implementation for RDF-based RSS 0.9 and RSS 1.0 feeds.
 *
 * @author Frank Osterfeld
 */
class Parser : public Syndication::AbstractParser
{
public:
    /** default constructor */
    Parser();

    /** destructor */
    ~Parser() override;

    /**
     * returns whether the passed document looks like
     * an RSS 0.9 or RSS 1.0 document.
     * @param source the document source to check
     */
    bool accept(const DocumentSource &source) const override;

    /**
     * Parses an RSS 0.9/1.0 document from a feed source. RSS 0.9
     * documents are converted to RSS 1.0.
     *
     * @param source The document source to parse
     * @return parsed document (a Syndication::RDF::Document), or an
     * invalid document if parsing failed.
     * @see Document::isValid()
     */
    SpecificDocumentPtr parse(const DocumentSource &source) const override;

    /**
     * format string of this parser, which is @c "rdf".
     *
     * @return @c "rdf"
     */
    QString format() const override;

private:
    Parser(const Parser &other);
    Parser &operator=(const Parser &other);

    class ParserPrivate;
    std::unique_ptr<ParserPrivate> const d;
};

} // namespace RDF
} // namespace Syndication

#endif // SYNDICATION_RDF_PARSER_H
