/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RSS2_PARSER_H
#define SYNDICATION_RSS2_PARSER_H

#include <syndication/abstractparser.h>
#include <syndication/rss2/document.h>

#include <memory>

namespace Syndication
{
class DocumentSource;

namespace RSS2
{
class Document;

/**
 * Parser implementation for the RSS 0.9x/2.0 format family
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
     * checks whether a document source looks like an RSS0.9x/2.0 document
     *
     * @param source a document source to check
     * @return @c true if the source looks like an RSS2 document
     */
    bool accept(const DocumentSource &source) const override;

    /**
     * creates an RSS2 wrapper for a document source.
     * The wrapper will only return useful values if the source is really
     * RSS 0.9x/2.0, so call accept() before to check.
     *
     * @param source the source to wrap
     * @return A Syndication::RSS2::Document instance wrapping the XML
     * source, or a null document (not a null pointer!) if there is no @c
     * &lt;channel> root element in the source.
     */
    Syndication::SpecificDocumentPtr parse(const DocumentSource &source) const override;

    /**
     * returns the format string of this parser implementation, which is
     * "rss2".
     *
     * @return @c "rss2"
     */
    QString format() const override;

private:
    Parser(const Parser &other);
    Parser &operator=(const Parser &other);

    class ParserPrivate;
    std::unique_ptr<ParserPrivate> const d;
};

} // namespace RSS2
} // namespace Syndication

#endif // SYNDICATION_RSS2_PARSER_H
